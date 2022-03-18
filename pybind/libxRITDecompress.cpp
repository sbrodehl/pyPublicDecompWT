#include <iostream>
#include <istream>
#include <streambuf>
#include <cstring>
#include <memory>

#include <pybind11/pybind11.h>

#include "CxRITFile.h"				// DISE
#include "CxRITFileDecompressed.h"	// DISE


namespace py = pybind11;

const char* VERSION_NUMBER="2.8.1";

class xRITWrapper {
public:
    xRITWrapper(char* in_buffer, int in_bytes);
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

const char *xRITWrapper::getAnnotationText() const {
    return file_annotation.c_str();
}

unsigned long long xRITWrapper::getOutputLength() const {
    return output_length;
}

py::bytes xRITWrapper::data() const {
    return py::bytes(output_buffer.str());
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

const char *xRITWrapper::getTimeStamp() const {
    return utctime_format.c_str();
}

int xRITWrapper::getFileTypeCode() const {
    return fileTypeCode;
}

PYBIND11_MODULE(pyxRITDecompress, m) {
    py::class_<xRITWrapper>(m, "xRITWrapper", py::dynamic_attr())
            .def(py::init<char *, int>())
            .def("getAnnotationText", &xRITWrapper::getAnnotationText)
            .def("getOutputLength", &xRITWrapper::getOutputLength)
            .def("data", &xRITWrapper::data)
            .def("getTotalHeaderLength", &xRITWrapper::getTotalHeaderLength)
            .def("getSpectralChannelID", &xRITWrapper::getSpectralChannelID)
            .def("getSegmentSeqNo", &xRITWrapper::getSegmentSeqNo)
            .def("getTimeStamp", &xRITWrapper::getTimeStamp)
            .def("getFileTypeCode", &xRITWrapper::getFileTypeCode);
    m.attr("__version__") = VERSION_NUMBER;
}
