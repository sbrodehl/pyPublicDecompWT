set(LIBNAME "COMP")

include_directories(Src)
include_directories(Inc)

set(SOURCE_FILES
        Src/CBuffer.cpp
        Src/CImage.cpp
        Src/CQualityMeasures.cpp
        )

add_library(${LIBNAME} STATIC ${SOURCE_FILES})
set_property(TARGET ${LIBNAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${LIBNAME} LINK_PUBLIC "DISE")
target_include_directories(${LIBNAME} PUBLIC Inc)
set_property(TARGET ${LIBNAME} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
        $<INSTALL_INTERFACE:Inc>  # <prefix>/Inc
        )
install(TARGETS ${LIBNAME} DESTINATION lib)


# add subdirs
add_subdirectory(T4)
add_subdirectory(JPEG)
add_subdirectory(WT)
