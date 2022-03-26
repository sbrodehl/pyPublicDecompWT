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
    explicit xRITDecompress(const py::bytes& in_buffer);
    unsigned long long getOutputLength() const;
    py::bytes data() const;
    unsigned long long getTotalHeaderLength() const;
    int getSpectralChannelID() const;
    int getSegmentSeqNo() const;
    int getFileTypeCode() const;
    const char *getTimeStamp() const;
    const char *getAnnotationText() const;
private:
    std::ostringstream output_buffer;
    unsigned long long output_length = 0;
    unsigned long long totalHeaderLength = 0;
    int spectralChannelID = 0;
    int segmentSeqNo = -1;
    int fileTypeCode = -1;
    std::string file_annotation;
    std::string utctime_format;
};

xRITDecompress::xRITDecompress(const py::bytes& in_buffer) {
    std::istringstream in_stream(in_buffer);
    // Open input file.
    DISE::CxRITFile compressedFile;
    try {
        compressedFile = DISE::CxRITFile(in_stream);
    } catch (...) {
        std::cerr << "Opening/reading input file failed.\n";
    }
    // Decompress input file.
    DISE::CxRITFileDecompressed decompressedFile(compressedFile);
    spectralChannelID = decompressedFile.GetSpectralChannelID().m_SC_CHAN_ID;
    segmentSeqNo = decompressedFile.GetSegmentSeqNo();
    totalHeaderLength = decompressedFile.GetTotalHeaderLength();
    fileTypeCode = decompressedFile.GetFileTypeCode();
    auto utctime(const_cast<SYSTIME &>(decompressedFile.GetTimeStamp()));
#ifdef WIN32
    utctime_format = utctime.FormatDate("yyyyMMdd") + utctime.FormatTime("HHmmss");
#else
    utctime_format = utctime.Format("%Y%m%d%H%M%S");
#endif
    file_annotation = decompressedFile.GetAnnotation().GetText();
    decompressedFile.Write(output_buffer);
    output_buffer.seekp(0, std::ios::end);
    output_length = output_buffer.tellp();
}

const char *xRITDecompress::getAnnotationText() const {
    return file_annotation.c_str();
}

unsigned long long xRITDecompress::getOutputLength() const {
    return output_length;
}

py::bytes xRITDecompress::data() const {
    return py::bytes(output_buffer.str());
}

unsigned long long xRITDecompress::getTotalHeaderLength() const {
    return totalHeaderLength;
}

int xRITDecompress::getSpectralChannelID() const {
    return spectralChannelID;
}

int xRITDecompress::getSegmentSeqNo() const {
    return segmentSeqNo;
}

const char *xRITDecompress::getTimeStamp() const {
    return utctime_format.c_str();
}

int xRITDecompress::getFileTypeCode() const {
    return fileTypeCode;
}

PYBIND11_MODULE(pyPublicDecompWT, m) {
    py::class_<xRITDecompress>(m, "xRITDecompress", py::dynamic_attr())
            .def(py::init<py::bytes&>())
            .def("getAnnotationText", &xRITDecompress::getAnnotationText)
            .def("getOutputLength", &xRITDecompress::getOutputLength)
            .def("data", &xRITDecompress::data)
            .def("getTotalHeaderLength", &xRITDecompress::getTotalHeaderLength)
            .def("getSpectralChannelID", &xRITDecompress::getSpectralChannelID)
            .def("getSegmentSeqNo", &xRITDecompress::getSegmentSeqNo)
            .def("getTimeStamp", &xRITDecompress::getTimeStamp)
            .def("getFileTypeCode", &xRITDecompress::getFileTypeCode);
#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(PYXRITDECOMPRESS_VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif

}
