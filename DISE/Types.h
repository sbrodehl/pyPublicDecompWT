#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef WIN32
#pragma message("WINDOWS BUILD")
#include "MSGTime.h"
typedef Util::CMSGTime SYSTIME;
typedef Util::CMSGTimeSpan SYSTIMESPAN;
#else
#include "UTCTime.h"
typedef  Util::CUTCTime SYSTIME;
typedef  Util::CTimeSpan SYSTIMESPAN;
#define __int64 long long
#define __int32 int
#define __int16 short
#define __int8 char
#endif

#endif
