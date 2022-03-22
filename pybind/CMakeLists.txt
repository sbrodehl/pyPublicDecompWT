set(LIBNAME "pyPublicDecompWT")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES libxRITDecompress.cpp)

pybind11_add_module(${LIBNAME} MODULE ${SOURCE_FILES})
target_link_libraries(${LIBNAME} PRIVATE "DISE" "COMP" "JPEG" "WT" "T4")
target_compile_definitions(${LIBNAME} PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS ${LIBNAME} DESTINATION .)
