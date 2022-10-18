set(LIBNAME "DISE")

include_directories(.)

set(SOURCE_FILES
        CxRITFile.h
        CxRITFile.cpp
        CxRITFileHeaderRecords.h
        CxRITFileHeaderRecords.cpp
        CxRITAnnotation.h
        CxRITAnnotation.cpp
        ErrorHandling.h
        ErrorHandling.cpp
        Types.h
        )

if (NOT WIN32) 
SET(SOURCE_FILES ${SOURCE_FILES} 
        TimeSpan.h
        TimeSpan.cpp
        UTCTime.h
        UTCTime.cpp
        )
else()
SET(SOURCE_FILES ${SOURCE_FILES}
        MSGTime.h
        MSGTime_impl.h
        )
endif()

add_library(${LIBNAME} STATIC ${SOURCE_FILES})
set_property(TARGET ${LIBNAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${LIBNAME} PUBLIC .)
set_property(TARGET ${LIBNAME} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
        $<INSTALL_INTERFACE:>  # <prefix>/
        )
install(TARGETS ${LIBNAME} DESTINATION lib)
