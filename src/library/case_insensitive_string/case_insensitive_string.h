#pragma once

#include "case_insensitive_char_traits.h"

#include <src/util/string/split.h>

#include <string>
#include <string_view>

using TCaseInsensitiveString = std::basic_string<char, TCaseInsensitiveCharTraits>;
using TCaseInsensitiveStringBuf = std::basic_string_view<char, TCaseInsensitiveCharTraits>;

template <>
struct THash<TCaseInsensitiveStringBuf> {
    size_t operator()(TCaseInsensitiveStringBuf str) const noexcept;
};

template <>
struct THash<TCaseInsensitiveString> : THash<TCaseInsensitiveStringBuf> {};

namespace NStringSplitPrivate {

    template<>
    struct TStringBufOfImpl<TCaseInsensitiveStringBuf> {
        /*
         * WARN:
         * StringSplitter does not use TCharTraits properly.
         * Splitting such strings is explicitly disabled.
         */
        // using type = TCaseInsensitiveStringBuf;
    };

    template<>
    struct TStringBufOfImpl<TCaseInsensitiveString> : TStringBufOfImpl<TCaseInsensitiveStringBuf> {
    };

} // namespace NStringSplitPrivate
