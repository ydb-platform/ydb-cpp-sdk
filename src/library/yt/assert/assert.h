#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compiler.h>
#include <ydb-cpp-sdk/util/system/src_root.h>
=======
#include <src/util/system/compiler.h>
#include <src/util/system/src_root.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/compiler.h>
#include <ydb-cpp-sdk/util/system/src_root.h>
>>>>>>> origin/main

#include <string_view>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

namespace NDetail {

[[noreturn]]
void AssertTrapImpl(
    std::string_view trapType,
    std::string_view expr,
    std::string_view file,
    int line,
    std::string_view function);

} // namespace NDetail

#ifdef __GNUC__
    #define YT_BUILTIN_TRAP()  __builtin_trap()
#else
    #define YT_BUILTIN_TRAP()  std::terminate()
#endif

#define YT_ASSERT_TRAP(trapType, expr) \
    ::NYT::NDetail::AssertTrapImpl(std::string_view(trapType), std::string_view(expr), __SOURCE_FILE_IMPL__.As<std::string_view>(), __LINE__, std::string_view(__FUNCTION__)); \
    Y_UNREACHABLE() \

#ifdef NDEBUG
    #define YT_ASSERT(expr) \
        do { \
            if (false) { \
                (void) (expr); \
            } \
        } while (false)
#else
    #define YT_ASSERT(expr) \
        do { \
            if (Y_UNLIKELY(!(expr))) { \
                YT_ASSERT_TRAP("YT_ASSERT", #expr); \
            } \
        } while (false)
#endif

//! Same as |YT_ASSERT| but evaluates and checks the expression in both release and debug mode.
#define YT_VERIFY(expr) \
    do { \
        if (Y_UNLIKELY(!(expr))) { \
            YT_ASSERT_TRAP("YT_VERIFY", #expr); \
        } \
    } while (false)

//! Fatal error code marker. Abnormally terminates the current process.
#ifdef YT_COMPILING_UDF
    #define YT_ABORT() __YT_BUILTIN_ABORT()
#else
    #define YT_ABORT() \
        do { \
            YT_ASSERT_TRAP("YT_ABORT", ""); \
        } while (false)
#endif

//! Unimplemented code marker. Abnormally terminates the current process.
#define YT_UNIMPLEMENTED() \
    do { \
        YT_ASSERT_TRAP("YT_UNIMPLEMENTED", ""); \
    } while (false)

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
