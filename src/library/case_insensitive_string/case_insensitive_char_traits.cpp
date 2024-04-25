#include "case_insensitive_char_traits.h"
#include "case_insensitive_string.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/escape.h>
=======
#include <src/util/string/escape.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/escape.h>
>>>>>>> origin/main

int TCaseInsensitiveCharTraits::compare(const char* s1, const char* s2, std::size_t n) {
    while (n-- != 0) {
        if (to_upper(*s1) < to_upper(*s2)) {
            return -1;
        }
        if (to_upper(*s1) > to_upper(*s2)) {
            return 1;
        }
        ++s1;
        ++s2;
    }
    return 0;
}

const char* TCaseInsensitiveCharTraits::find(const char* s, std::size_t n, char a) {
    auto const ua(to_upper(a));
    while (n-- != 0) {
        if (to_upper(*s) == ua)
            return s;
        s++;
    }
    return nullptr;
}

TCaseInsensitiveString EscapeC(const TCaseInsensitiveString& str) {
    const auto result = EscapeC(str.data(), str.size());
    return {result.data(), result.size()};
}

