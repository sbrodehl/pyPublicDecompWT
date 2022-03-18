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

const char* VERSION_NUMBER="2.8.1";

class xRITWrapper {
public:
    xRITWrapper(char* in_buffer, int in_bytes);
    unsigned long long getOutputLength() const;
    void write(char* return_buffer);
    unsigned long long getTotalHeaderLength() const;
    int getSpectralChannelID() const;
    int getSegmentSeqNo() const;
    int getFileTypeCode() const;
    void getTimeStamp(char* buf);
    void getAnnotationText(char* text);
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

xRITWrapper::xRITWrapper(char *in_buffer, int in_bytes) {
    std::istringstream in_stream(std::string(in_buffer, in_bytes));
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
    utctime_format = utctime.Format("%Y%m%d%H%M%S");
    file_annotation = decompressedFile.GetAnnotation().GetText();
    decompressedFile.Write(output_buffer);
    output_buffer.seekp(0, std::ios::end);
    output_length = output_buffer.tellp();
}

void xRITWrapper::getAnnotationText(char *text) {
    std::memcpy(text, file_annotation.c_str(), file_annotation.length());
}

unsigned long long xRITWrapper::getOutputLength() const {
    return output_length;
}

void xRITWrapper::write(char* return_buffer) {
    std::memcpy(return_buffer, output_buffer.str().c_str(), output_length);
}

unsigned long long xRITWrapper::getTotalHeaderLength() const {
    return totalHeaderLength;
}

int xRITWrapper::getSpectralChannelID() const {
    return spectralChannelID;
}

int xRITWrapper::getSegmentSeqNo() const {
    return segmentSeqNo;
}

void xRITWrapper::getTimeStamp(char *buf) {
    std::memcpy(buf, utctime_format.c_str(), 14);
}

int xRITWrapper::getFileTypeCode() const {
    return fileTypeCode;
}

PYBIND11_MODULE(pyxRITDecompress, m) {
    py::class_<xRITWrapper>(m, "xRITWrapper", py::dynamic_attr())
            .def(py::init<char *, int>())
            .def("getAnnotationText", &xRITWrapper::getAnnotationText)
            .def("getOutputLength", &xRITWrapper::getOutputLength)
            .def("write", &xRITWrapper::write)
            .def("getTotalHeaderLength", &xRITWrapper::getTotalHeaderLength)
            .def("getSpectralChannelID", &xRITWrapper::getSpectralChannelID)
            .def("getSegmentSeqNo", &xRITWrapper::getSegmentSeqNo)
            .def("getTimeStamp", &xRITWrapper::getTimeStamp)
            .def("getFileTypeCode", &xRITWrapper::getFileTypeCode);
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_NUMBER);
}
