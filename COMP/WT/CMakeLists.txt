set(LIBNAME "WT")

include_directories(Src)
include_directories(Inc)

set(SOURCE_FILES
        Src/CACCoder.cpp
        Src/CACDecoder.cpp
        Src/CACModel.cpp
        Src/CVLCCoder.cpp
        Src/CVLCDecoder.cpp
        Src/CWBlock.cpp
        Src/CWTCoder.cpp
        Src/CWTDecoder.cpp
        Src/CompressWT.cpp
        )

add_library(${LIBNAME} SHARED ${SOURCE_FILES})
target_link_libraries(${LIBNAME} LINK_PUBLIC "DISE" "COMP")
target_include_directories(${LIBNAME} PUBLIC Inc)
set_property(TARGET ${LIBNAME} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
        $<INSTALL_INTERFACE:Inc>  # <prefix>/Inc
        )
