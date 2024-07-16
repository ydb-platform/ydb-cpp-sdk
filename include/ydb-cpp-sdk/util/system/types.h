#pragma once

// DO_NOT_STYLE

#include <ydb-cpp-sdk/util/system/platform.h>

#include <inttypes.h>

using i8 = int8_t;
using i16 = int16_t;
using ui8 = uint8_t;
using ui16 = uint16_t;

using yssize_t = int;
#define PRIYSZT "d"

#if defined(_darwin_) && defined(_32_)
using ui32 = unsigned long;
using i32 = long;
#else
using ui32 = uint32_t;
using i32 = int32_t;
#endif

#if defined(_darwin_) && defined(_64_)
using ui64 = unsigned long;
using i64 = long;
#else
using ui64 = uint64_t;
using i64 = int64_t;
#endif

#define LL(number) INT64_C(number)
#define ULL(number) UINT64_C(number)

// Macro for size_t and ptrdiff_t types
#if defined(_32_)
    #if defined(_darwin_)
        #define PRISZT "lu"
        #undef PRIi32
        #define PRIi32 "li"
        #undef SCNi32
        #define SCNi32 "li"
        #undef PRId32
        #define PRId32 "li"
        #undef SCNd32
        #define SCNd32 "li"
        #undef PRIu32
        #define PRIu32 "lu"
        #undef SCNu32
        #define SCNu32 "lu"
        #undef PRIx32
        #define PRIx32 "lx"
        #undef SCNx32
        #define SCNx32 "lx"
    #elif !defined(_cygwin_)
        #define PRISZT PRIu32
    #else
        #define PRISZT "u"
    #endif
    #define SCNSZT SCNu32
    #define PRIPDT PRIi32
    #define SCNPDT SCNi32
    #define PRITMT PRIi32
    #define SCNTMT SCNi32
#elif defined(_64_)
    #if defined(_darwin_)
        #define PRISZT "lu"
        #undef PRIu64
        #define PRIu64 PRISZT
        #undef PRIx64
        #define PRIx64 "lx"
        #undef PRIX64
        #define PRIX64 "lX"
        #undef PRId64
        #define PRId64 "ld"
        #undef PRIi64
        #define PRIi64 "li"
        #undef SCNi64
        #define SCNi64 "li"
        #undef SCNu64
        #define SCNu64 "lu"
        #undef SCNx64
        #define SCNx64 "lx"
    #else
        #define PRISZT PRIu64
    #endif
    #define SCNSZT SCNu64
    #define PRIPDT PRIi64
    #define SCNPDT SCNi64
    #define PRITMT PRIi64
    #define SCNTMT SCNi64
#else
    #error "Unsupported platform"
#endif

// SUPERLONG
#if !defined(DONT_USE_SUPERLONG) && !defined(SUPERLONG_MAX)
    #define SUPERLONG_MAX ~LL(0)
using SUPERLONG = i64;
#endif

// UNICODE
#ifdef __cplusplus
// UCS-2, native byteorder
using wchar16 = char16_t;
// internal symbol type: UTF-16LE
using TChar = wchar16;
using wchar32 = char32_t;
#endif

#if defined(_MSC_VER)
    #include <basetsd.h>
typedef SSIZE_T ssize_t;
    #define HAVE_SSIZE_T 1
    #include <wchar.h>
#endif

#include <sys/types.h>

// for pb.h/cc
using arc_i32 = i32;
using arc_i64 = i64;
using arc_ui32 = ui32;
using arc_ui64 = ui64;
