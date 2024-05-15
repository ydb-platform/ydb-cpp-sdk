#include "case_insensitive_char_traits.h"
#include "case_insensitive_string.h"

#include <ydb-cpp-sdk/util/string/escape.h>

const TCaseInsensitiveCharTraits::char_type* TCaseInsensitiveCharTraits::find(
        const char_type* s, std::size_t n, char_type a) noexcept(IsNothrowToUpper) {
    const auto ua = to_upper(a);
    for (; n != 0; --n, ++s) {
        if (to_upper(*s) == ua) {
            return s;
        }
    }
    return nullptr;
}

TCaseInsensitiveString EscapeC(const TCaseInsensitiveString& str) {
    const auto result = EscapeC(str.data(), str.size());
    return {result.data(), result.size()};
}
