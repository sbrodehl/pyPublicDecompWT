#include <iostream>
#include <istream>
#include <limits>
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

// The core parser reads sequentially. Exposing only the get area lets it read
// the Python bytes directly without copying them into a temporary std::string.
// The argument owns the memory for the duration of decompress(), with the GIL
// held throughout; this stream never writes to that immutable input.
class InputBuffer : public std::streambuf {
public:
    InputBuffer(char *data, std::size_t size) {
        setg(data, data, data + size);
    }
};

// Write into a fresh, private Python bytes allocation. The bounded put area
// makes an unexpected length mismatch fail instead of overrunning the buffer.
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
    py::object output_cache;
    DISE::CxRITFileDecompressed decompressed_file = DISE::CxRITFileDecompressed(DISE::CxRITFile());
    unsigned long long output_length = -1;
    unsigned long long total_header_length = -1;
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
    decompressed_file.Write(file_annotation);
}

void xRITDecompress::decompress(const py::bytes& in_buffer) {
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
            std::cerr << "Input file of buffer not specified.\n";
        }
    } catch (...) {
        std::cerr << "Opening/reading input file failed.\n";
    }
    decompressed_file = DISE::CxRITFileDecompressed(compressedFile);
    output_cache = py::object();
    output_length = -1;
    spectral_channel_ID = decompressed_file.GetSpectralChannelID().m_SC_CHAN_ID;
    segment_seq_no = decompressed_file.GetSegmentSeqNo();
    total_header_length = decompressed_file.GetTotalHeaderLength();
    file_type_code = decompressed_file.GetFileTypeCode();
    auto utctime(const_cast<SYSTIME &>(decompressed_file.GetTimeStamp()));
#ifdef WIN32
    utctime_format = utctime.FormatDate("yyyyMMdd") + utctime.FormatTime("HHmmss");
#else
    utctime_format = utctime.Format("%Y%m%d%H%M%S");
#endif
    file_annotation = decompressed_file.GetAnnotation().GetText();
}

const char *xRITDecompress::getAnnotationText() const {
    return file_annotation.c_str();
}

unsigned long long xRITDecompress::getOutputLength() const {
    return output_length;
}

py::bytes xRITDecompress::data() {
    if (!output_cache) {
        const auto bits = decompressed_file.GetTotalLength();
        const auto size = bits / 8 + (bits % 8 != 0);
        if (size > static_cast<unsigned long long>(std::numeric_limits<Py_ssize_t>::max())) {
            throw std::overflow_error("Decompressed output is too large for Python bytes.");
        }

        auto result = py::reinterpret_steal<py::bytes>(
                PyBytes_FromStringAndSize(nullptr, static_cast<Py_ssize_t>(size)));
        if (!result) {
            throw py::error_already_set();
        }
        char *output_data = PyBytes_AsString(result.ptr());
        if (!output_data) {
            throw py::error_already_set();
        }
        OutputBuffer output_buffer(output_data, static_cast<std::size_t>(size));
        std::ostream out_stream(&output_buffer);
        decompressed_file.Write(out_stream);
        if (output_buffer.size() != size) {
            throw std::runtime_error("Serialized output length does not match the file headers.");
        }

        output_length = size;
        output_cache = std::move(result);
    }
    return py::reinterpret_borrow<py::bytes>(output_cache);
}

unsigned long long xRITDecompress::getTotalHeaderLength() const {
    return total_header_length;
}

int xRITDecompress::getSpectralChannelID() const {
    return spectral_channel_ID;
}

int xRITDecompress::getSegmentSeqNo() const {
    return segment_seq_no;
}

const char *xRITDecompress::getTimeStamp() const {
    return utctime_format.c_str();
}

int xRITDecompress::getFileTypeCode() const {
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
            .def("getOutputLength", &xRITDecompress::getOutputLength, "Returns the length of the decompressed output.")
            .def("decompress", &xRITDecompress::decompress, "Decompresses file or given buffer.", pybind11::arg("in_buffer") = py::bytes())
            .def("data", &xRITDecompress::data, "Returns the decompressed output.")
            .def("getTotalHeaderLength", &xRITDecompress::getTotalHeaderLength, "Returns the accumulated length of all header records.")
            .def("getSpectralChannelID", &xRITDecompress::getSpectralChannelID, "Returns the SEVIRI spectral channel identifier.")
            .def("getSegmentSeqNo", &xRITDecompress::getSegmentSeqNo, "Returns the segment sequence number.")
            .def("getTimeStamp", &xRITDecompress::getTimeStamp, "Returns the Time Stamp Header Record.")
            .def("getFileTypeCode", &xRITDecompress::getFileTypeCode, "Returns the File Type Code Header Record.");
#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
