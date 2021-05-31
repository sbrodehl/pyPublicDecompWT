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
    void getAnnotationText(char* text);
private:
    std::ostringstream output_buffer;
    std::string file_annotation;
    unsigned long long output_length = 0;
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
    file_annotation = decompressedFile.GetAnnotation().GetText();
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

void xRITWrapper::getAnnotationText(char *text) {
    std::memcpy(text, file_annotation.c_str(), 61);
}


// Define C functions for the C++ class - as ctypes can only talk to C...
EXTERN_DLL_EXPORT {
    xRITWrapper* xRITWrapper_Constructor(char* in_buffer, int in_bytes) {
        return new xRITWrapper(in_buffer, in_bytes);
    }
    unsigned long long xRITWrapper_getOutputLength(xRITWrapper* foo) {
        return foo->getOutputLength();
    }
    void xRITWrapper_write(xRITWrapper* foo, char* out_buffer) {
        return foo->write(out_buffer);
    }
    void xRITWrapper_getAnnotationText(xRITWrapper* foo, char* text) {
        return foo->getAnnotationText(text);
    }
}