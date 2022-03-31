#include <iostream>
#include <istream>
#include <streambuf>
#include <cstring>
#include <memory>

#include <pybind11/pybind11.h>

#include "CxRITFile.h"				// DISE
#include "CxRITFileDecompressed.h"	// DISE

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

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
    std::ostringstream output_buffer;
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
    try {
        if (len(in_buffer) > 0) {
            std::istringstream in_stream(in_buffer);
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
    decompressed_file.Write(output_buffer);
    output_buffer.seekp(0, std::ios::end);
    output_length = output_buffer.tellp();
    return py::bytes(output_buffer.str());
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
            .def("decompress", &xRITDecompress::decompress, "Decompresses file or given buffer.", pybind11::arg("in_buffer") = "")
            .def("data", &xRITDecompress::data, "Returns the decompressed output.")
            .def("getTotalHeaderLength", &xRITDecompress::getTotalHeaderLength, "Returns the accumulated length of all header records.")
            .def("getSpectralChannelID", &xRITDecompress::getSpectralChannelID, "Returns the SEVIRI spectral channel identifier.")
            .def("getSegmentSeqNo", &xRITDecompress::getSegmentSeqNo, "Returns the segment sequence number.")
            .def("getTimeStamp", &xRITDecompress::getTimeStamp, "Returns the Time Stamp Header Record.")
            .def("getFileTypeCode", &xRITDecompress::getFileTypeCode, "Returns the File Type Code Header Record.");
#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(PYXRITDECOMPRESS_VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
