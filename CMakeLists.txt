cmake_minimum_required(VERSION 4.2)
set(PROJNAME "PublicDecompWT")
project(${PROJNAME} VERSION "2.8.1")

# Keep temporary source directories out of __FILE__ and debug information.
include(CheckCXXCompilerFlag)

set(_file_prefix_map "-ffile-prefix-map=${PROJECT_SOURCE_DIR}/=./")
check_cxx_compiler_flag("${_file_prefix_map}" HAS_FFILE_PREFIX_MAP)

if(HAS_FFILE_PREFIX_MAP)
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${_file_prefix_map}>")
endif()

# Add verbose compile output
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Preserve the old CMake command-line option while using modern FindPython.
if(DEFINED PYTHON_EXECUTABLE AND NOT DEFINED Python_EXECUTABLE)
    set(Python_EXECUTABLE "${PYTHON_EXECUTABLE}")
endif()
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
set(PYBIND11_FINDPYTHON ON)

if(SKBUILD)
    # Scikit-Build does not add your site-packages to the search path
    # automatically, so we need to add it _or_ the pybind11 specific directory
    # here.
    execute_process(
            COMMAND "${Python_EXECUTABLE}" -c
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
