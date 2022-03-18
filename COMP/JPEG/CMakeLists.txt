set(LIBNAME "JPEG")

include_directories(Inc)

set(SOURCE_FILES
        Src/CHcodec.cpp
        Src/CHOptim.cpp
        Src/CHufftables.cpp
        Src/CJBlock.cpp
        Src/CJPEGDecoder.cpp
        Src/CJPEGLossLessCoder.cpp
        Src/CJPEGLossyCoder.cpp
        Src/CompressJPEG.cpp
        Src/CQuantizationTable.cpp
        )

add_library(${LIBNAME} SHARED ${SOURCE_FILES})
target_link_libraries(${LIBNAME} LINK_PUBLIC "DISE" "COMP")
target_include_directories(${LIBNAME} PUBLIC Inc)
set_property(TARGET ${LIBNAME} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
        $<INSTALL_INTERFACE:Inc>  # <prefix>/Inc
        )
install(TARGETS ${LIBNAME} DESTINATION .)
