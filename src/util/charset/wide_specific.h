#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
=======
#include <src/util/system/types.h>
#include <src/util/system/yassert.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

inline constexpr bool IsW16SurrogateLead(wchar16 c) noexcept {
    return 0xD800 <= c && c <= 0xDBFF;
}

inline constexpr bool IsW16SurrogateTail(wchar16 c) noexcept {
    return 0xDC00 <= c && c <= 0xDFFF;
}

inline size_t W16SymbolSize(const wchar16* begin, const wchar16* end) {
    Y_ASSERT(begin < end);

    if ((begin + 1 != end) && IsW16SurrogateLead(*begin) && IsW16SurrogateTail(*(begin + 1))) {
        return 2;
    }

    return 1;
}
