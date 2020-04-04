cmake_minimum_required(VERSION 3.12)
set(PROJNAME "PublicDecompWT")
project(${PROJNAME})

# Add verbose compile output
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# a common place where CMake should put all executable / library files
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/dist/bin)
# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/dist/lib)

find_package(Threads REQUIRED)

# add subdirs
add_subdirectory(COMP)
add_subdirectory(DISE)
add_subdirectory(xRITDecompress)
