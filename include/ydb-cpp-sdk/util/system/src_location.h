#pragma once

#include <ydb-cpp-sdk/util/system/src_root.h>

#include <string_view>

struct TSourceLocation {
    constexpr TSourceLocation(const std::string_view f, int l) noexcept
        : File(f)
        , Line(l)
    {
    }

    std::string_view File;
    int Line;
};

// __SOURCE_FILE__ should be used instead of __FILE__
#if !defined(__NVCC__)
    #define __SOURCE_FILE__ (__SOURCE_FILE_IMPL__.As<std::string_view>())
#else
    #define __SOURCE_FILE__ (__SOURCE_FILE_IMPL__.template As<std::string_view>())
#endif

#define __LOCATION__ ::TSourceLocation(__SOURCE_FILE__, __LINE__)
