#include <iostream>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <streambuf>
#include <utility>

#include <pybind11/pybind11.h>

#include "CxRITFile.h"				// DISE
#include "CxRITFileDecompressed.h"	// DISE

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

namespace {

#if PY_VERSION_HEX >= 0x030F0000
class PythonBytesOutput {
public:
    explicit PythonBytesOutput(Py_ssize_t size)
        : writer(PyBytesWriter_Create(size)) {
        if (!writer) {
            throw py::error_already_set();
        }
    }

    char *data() const {
        return static_cast<char *>(PyBytesWriter_GetData(writer.get()));
    }

    py::bytes finish() {
        auto result = py::reinterpret_steal<py::bytes>(
                PyBytesWriter_Finish(writer.release()));
        if (!result) {
            throw py::error_already_set();
        }
        return result;
    }

private:
    struct DiscardWriter {
        void operator()(PyBytesWriter *value) const {
            PyBytesWriter_Discard(value);
        }
    };
    std::unique_ptr<PyBytesWriter, DiscardWriter> writer;
};
#else
class PythonBytesOutput {
public:
    explicit PythonBytesOutput(Py_ssize_t size)
        : result(py::reinterpret_steal<py::bytes>(
                PyBytes_FromStringAndSize(nullptr, size))) {
        if (!result) {
            throw py::error_already_set();
        }
    }

    char *data() const {
        char *buffer = PyBytes_AsString(result.ptr());
        if (!buffer) {
            throw py::error_already_set();
        }
        return buffer;
    }

    py::bytes finish() {
        return std::move(result);
    }

private:
    py::bytes result;
};
#endif


class InputBuffer : public std::streambuf {
public:
    InputBuffer(char *data, std::size_t size) {
        setg(data, data, data + size);
    }
};

class OutputBuffer : public std::streambuf {
public:
    OutputBuffer(char *data, std::size_t size) {
        setp(data, data + size);
    }

    std::size_t size() const {
        return static_cast<std::size_t>(pptr() - pbase());
    }
};

} // namespace

class xRITDecompress {
public:
    explicit xRITDecompress(std::string file_path = "");
    void decompress(const py::bytes& in_buffer = "");
    void to_file();
    unsigned long long getOutputLength() const;
    py::bytes data();
    unsigned long long getTotalHeaderLength() const;
    int getSpectralChannelID() const;
    int getSegmentSeqNo() const;
    int getFileTypeCode() const;
    const char *getTimeStamp() const;
    const char *getAnnotationText() const;
private:
    void require_decompressed() const;
    py::object output_cache;
    std::unique_ptr<DISE::CxRITFileDecompressed> decompressed_file;
    unsigned long long output_length = 0;
    unsigned long long total_header_length = 0;
    int spectral_channel_ID = 0;
    int segment_seq_no = -1;
    int file_type_code = -1;
    std::string file_name;
    std::string file_annotation;
    std::string utctime_format;
};

xRITDecompress::xRITDecompress(std::string file_path): file_name(std::move(file_path)) {
    if (!file_name.empty()) {
        decompress();
        to_file();
    }
}

void xRITDecompress::to_file() {
    require_decompressed();
    decompressed_file->Write(file_annotation);
}

void xRITDecompress::require_decompressed() const {
    if (!decompressed_file) {
        throw std::runtime_error("No decompressed result is available.");
    }
}

void xRITDecompress::decompress(const py::bytes& in_buffer) {
    output_cache = py::object();
    decompressed_file.reset();
    DISE::CxRITFile compressedFile;
    char *input_data;
    Py_ssize_t input_size;
    if (PyBytes_AsStringAndSize(in_buffer.ptr(), &input_data, &input_size) != 0) {
        throw py::error_already_set();
    }
    try {
        if (input_size > 0) {
            InputBuffer input_buffer(input_data, static_cast<std::size_t>(input_size));
            std::istream in_stream(&input_buffer);
            compressedFile = DISE::CxRITFile(in_stream);
        } else if (!file_name.empty()) {
            compressedFile = DISE::CxRITFile(file_name);
        } else {
            throw std::invalid_argument("Input file or buffer not specified.");
        }
    } catch (const std::invalid_argument&) {
        throw;
    } catch (...) {
        throw std::runtime_error("Opening/reading input file failed.");
    }
    auto decoded = std::make_unique<DISE::CxRITFileDecompressed>(compressedFile);
    const auto bits = decoded->GetTotalLength();
    output_length = bits / 8 + (bits % 8 != 0);
    spectral_channel_ID = decoded->GetSpectralChannelID().m_SC_CHAN_ID;
    segment_seq_no = decoded->GetSegmentSeqNo();
    total_header_length = decoded->GetTotalHeaderLength();
    file_type_code = decoded->GetFileTypeCode();
    const NBO::TIME_CDS_SHORT cds_time(decoded->GetTimeStamp());
    const auto datetime = py::module_::import("datetime");
    const auto epoch = datetime.attr("datetime")(
        1958, 1, 1,
        py::arg("tzinfo") = datetime.attr("timezone").attr("utc")
    );
    const auto elapsed = datetime.attr("timedelta")(
        py::arg("days") = static_cast<unsigned short>(cds_time.m_Day),
        py::arg("milliseconds") = static_cast<unsigned long>(cds_time.m_MillisecondsOfDay)
    );
    utctime_format = (epoch + elapsed).attr("strftime")("%Y%m%d%H%M%S").cast<std::string>();
    file_annotation = decoded->GetAnnotation().GetText();
    decompressed_file = std::move(decoded);
}

const char *xRITDecompress::getAnnotationText() const {
    require_decompressed();
    return file_annotation.c_str();
}

unsigned long long xRITDecompress::getOutputLength() const {
    require_decompressed();
    return output_length;
}

py::bytes xRITDecompress::data() {
    require_decompressed();
    if (!output_cache) {
        const auto size = output_length;
        if (size > static_cast<unsigned long long>((std::numeric_limits<Py_ssize_t>::max)())) {
            throw std::overflow_error("Decompressed output is too large for Python bytes.");
        }

        PythonBytesOutput output(static_cast<Py_ssize_t>(size));
        {
            OutputBuffer output_buffer(output.data(), static_cast<std::size_t>(size));
            std::ostream out_stream(&output_buffer);
            decompressed_file->Write(out_stream);
            if (!out_stream || output_buffer.size() != size) {
                throw std::runtime_error("Serialized output length does not match the file headers.");
            }
        }
        auto result = output.finish();
        output_cache = std::move(result);
    }
    return py::reinterpret_borrow<py::bytes>(output_cache);
}

unsigned long long xRITDecompress::getTotalHeaderLength() const {
    require_decompressed();
    return total_header_length;
}

int xRITDecompress::getSpectralChannelID() const {
    require_decompressed();
    return spectral_channel_ID;
}

int xRITDecompress::getSegmentSeqNo() const {
    require_decompressed();
    return segment_seq_no;
}

const char *xRITDecompress::getTimeStamp() const {
    require_decompressed();
    return utctime_format.c_str();
}

int xRITDecompress::getFileTypeCode() const {
    require_decompressed();
    return file_type_code;
}

PYBIND11_MODULE(pyPublicDecompWT, m) {
    m.doc() = "pyPublicDecompWT (Python bindings for EUMETSAT's PublicDecompWT)"
              ""
              "pyPublicDecompWT provides Python bindings for some of EUMETSAT’s PublicDecompWT tools, such as xRITDecompress."
              "With pyPublicDecompWT compressed xRIT (HRIT/LRIT) files can now be decompressed in-memory, directly in Python!"
              ""
              "https://github.com/sbrodehl/pyPublicDecompWT";
    py::class_<xRITDecompress>(m, "xRITDecompress", py::dynamic_attr())
            .def(py::init<std::string&>(), "", pybind11::arg("file_path") = "")
            .def("getAnnotationText", &xRITDecompress::getAnnotationText, "Returns the complete Annotation Header Record.")
            .def("getOutputLength", &xRITDecompress::getOutputLength, "Returns the serialized output length in bytes, including headers.")
            .def("decompress", &xRITDecompress::decompress, "Decompresses file or given buffer.", pybind11::arg("in_buffer") = py::bytes())
            .def("data", &xRITDecompress::data, "Returns the decompressed output.")
            .def("getTotalHeaderLength", &xRITDecompress::getTotalHeaderLength, "Returns the accumulated length of all header records.")
            .def("getSpectralChannelID", &xRITDecompress::getSpectralChannelID, "Returns the SEVIRI spectral channel identifier.")
            .def("getSegmentSeqNo", &xRITDecompress::getSegmentSeqNo, "Returns the segment sequence number.")
            .def("getTimeStamp", &xRITDecompress::getTimeStamp, "Returns the Time Stamp Header Record in UTC as YYYYMMDDHHMMSS.")
            .def("getFileTypeCode", &xRITDecompress::getFileTypeCode, "Returns the File Type Code Header Record.");
#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
