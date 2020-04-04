set(LIBNAME "xRITDecompress")

include_directories(.)

set(SOURCE_FILES xRITDecompress.cpp)

add_executable(${LIBNAME} ${SOURCE_FILES})
target_link_libraries(${LIBNAME} LINK_PUBLIC "DISE" "COMP" "JPEG" "WT" "T4")
