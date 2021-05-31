#include <iostream>
#include <istream>
#include <streambuf>
#include <cstring>
#include <memory>

#include "CxRITFile.h"				// DISE
#include "CxRITFileDecompressed.h"	// DISE

const char* VERSION_NUMBER="2.7.2";

#define EXTERN_DLL_EXPORT extern "C"
#ifdef _WIN32
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
#endif

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
private:
    std::ostringstream output_buffer;
    unsigned long long output_length = 0;
    unsigned long long totalHeaderLength = 0;
    int spectralChannelID = 0;
    int segmentSeqNo = -1;
    int fileTypeCode = -1;
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
    decompressedFile.Write(output_buffer);
    output_buffer.seekp(0, std::ios::end);
    output_length = output_buffer.tellp();
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


// Define C functions for the C++ class - as ctypes can only talk to C...
EXTERN_DLL_EXPORT {
    xRITWrapper* xRITWrapper_Constructor(char* in_buffer, int in_bytes) {
        return new xRITWrapper(in_buffer, in_bytes);
    }
    int xRITWrapper_getSpectralChannelID(xRITWrapper* foo) {
        return foo->getSpectralChannelID();
    }
    int xRITWrapper_getFileTypeCode(xRITWrapper* foo) {
        return foo->getFileTypeCode();
    }
    int xRITWrapper_getSegmentSeqNo(xRITWrapper* foo) {
        return foo->getSegmentSeqNo();
    }
    unsigned long long xRITWrapper_getOutputLength(xRITWrapper* foo) {
        return foo->getOutputLength();
    }
    unsigned long long xRITWrapper_getTotalHeaderLength(xRITWrapper* foo) {
        return foo->getTotalHeaderLength();
    }
    void xRITWrapper_write(xRITWrapper* foo, char* buf) {
        return foo->write(buf);
    }
    void xRITWrapper_getTimeStamp(xRITWrapper* foo, char* buf) {
        return foo->getTimeStamp(buf);
    }
}