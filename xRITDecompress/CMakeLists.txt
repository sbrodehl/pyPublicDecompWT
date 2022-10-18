set(EXENAME "xRITDecompress")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

set(SOURCE_FILES xRITDecompress.cpp)

add_executable(${EXENAME} ${SOURCE_FILES})
target_link_libraries(${EXENAME} LINK_PUBLIC "DISE" "COMP" "JPEG" "WT" "T4")
install(TARGETS ${EXENAME} DESTINATION bin)
