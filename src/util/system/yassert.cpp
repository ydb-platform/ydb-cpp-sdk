#include <ydb-cpp-sdk/util/system/yassert.h>

#include <ydb-cpp-sdk/util/system/backtrace.h>
#include <ydb-cpp-sdk/util/system/guard.h>
#include <ydb-cpp-sdk/util/system/spinlock.h>
#include <ydb-cpp-sdk/util/system/src_root.h>

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/util/datetime/base.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/str.h>
<<<<<<< HEAD
=======
#include <src/util/datetime/base.h>
#include <src/util/generic/singleton.h>
#include <src/util/stream/output.h>
#include <src/util/stream/str.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

#include <cstdlib>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

#ifdef CLANG_COVERAGE
extern "C" {
    // __llvm_profile_write_file may not be provided if the executable target uses NO_CLANG_COVERAGE() macro and
    // arrives as test's dependency via DEPENDS() macro.
    // That's why we provide a weak no-op implementation for __llvm_profile_write_file,
    // which is used below in the code, to correctly save codecoverage profile before program exits using abort().
    Y_WEAK int __llvm_profile_write_file(void) {
        return 0;
    }
}

#endif

namespace {
    struct TPanicLockHolder: public TAdaptiveLock {
    };
}
namespace NPrivate {
    [[noreturn]] Y_NO_INLINE void InternalPanicImpl(int line, const char* function, const char* expr, int, int, int, const std::string_view file, const char* errorMessage, size_t errorMessageSize) noexcept;
}

void ::NPrivate::Panic(const TStaticBuf& file, int line, const char* function, const char* expr, const char* format, ...) noexcept {
    try {
        // We care of panic of first failed thread only
        // Otherwise stderr could contain multiple messages and stack traces shuffled
        auto guard = Guard(*Singleton<TPanicLockHolder>());

        std::string errorMsg;
        va_list args, args_copy;
        va_start(args, format);
        va_copy(args_copy, args);
        // format has " " prefix to mute GCC warning on empty format
        size_t len = std::vsprintf(nullptr, format[0] == ' ' ? format + 1 : format, args_copy);
        va_end(args_copy);
        errorMsg.resize(len + 1);
        std::vsnprintf(errorMsg.data(), len + 1, format[0] == ' ' ? format + 1 : format, args);
        errorMsg.resize(len);
        va_end(args);

        constexpr int abiPlaceholder = 0;
        ::NPrivate::InternalPanicImpl(line, function, expr, abiPlaceholder, abiPlaceholder, abiPlaceholder, file.As<std::string_view>(), errorMsg.c_str(), errorMsg.size());
    } catch (...) {
        // ¯\_(ツ)_/¯
    }

    abort();
}

namespace NPrivate {
    [[noreturn]] Y_NO_INLINE void InternalPanicImpl(int line, const char* function, const char* expr, int, int, int, const std::string_view file, const char* errorMessage, size_t errorMessageSize) noexcept try {
        std::string_view errorMsg{errorMessage, errorMessageSize};
        const std::string now = TInstant::Now().ToStringLocal();

        std::string r;
        TStringOutput o(r);
        if (expr) {
            o << "VERIFY failed (" << now << "): " << errorMsg << Endl;
        } else {
            o << "FAIL (" << now << "): " << errorMsg << Endl;
        }
        o << "  " << file << ":" << line << Endl;
        if (expr) {
            o << "  " << function << "(): requirement " << expr << " failed" << Endl;
        } else {
            o << "  " << function << "() failed" << Endl;
        }
        std::cerr << r << std::flush;
#ifndef WITH_VALGRIND
        PrintBackTrace();
#endif
#ifdef CLANG_COVERAGE
        if (__llvm_profile_write_file()) {
            std::cerr << "Failed to dump clang coverage" << std::endl;
        }
#endif
        abort();
    } catch (...) {
        abort();
    }
}
