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
        TimeSpan.h
        TimeSpan.cpp
        UTCTime.h
        UTCTime.cpp
        )

add_library(${LIBNAME} STATIC ${SOURCE_FILES})
target_include_directories(${LIBNAME} PUBLIC .)
