cmake_minimum_required(VERSION 3.15...3.19)
set(PROJNAME "PublicDecompWT")
project(${PROJNAME} VERSION "2.8.1")

# Add verbose compile output
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(SKBUILD)
    # Scikit-Build does not add your site-packages to the search path
    # automatically, so we need to add it _or_ the pybind11 specific directory
    # here.
    execute_process(
            COMMAND "${PYTHON_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_cmake_dir())"
            OUTPUT_VARIABLE _tmp_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
    list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")
find_package(Threads REQUIRED)
# Now we can find pybind11
find_package(pybind11 CONFIG REQUIRED)

# add subdirs
add_subdirectory(COMP)
add_subdirectory(DISE)
add_subdirectory(xRITDecompress)
add_subdirectory(pybind)
