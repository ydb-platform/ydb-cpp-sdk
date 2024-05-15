#pragma once

#include <ydb-cpp-sdk/util/system/compat.h>

#include <cctype>
#include <string>

struct TCaseInsensitiveCharTraits : private std::char_traits<char> {
    static bool eq(char c1, char c2) {
        return to_upper(c1) == to_upper(c2);
    }

    static bool lt(char c1, char c2) {
        return to_upper(c1) < to_upper(c2);
    }

    static int compare(const char* s1, const char* s2, std::size_t n) {
        return strnicmp(s1, s2, n);
    }

    static const char* find(const char* s, std::size_t n, char a);

    using std::char_traits<char>::assign;
    using std::char_traits<char>::char_type;
    using std::char_traits<char>::copy;
    using std::char_traits<char>::length;
    using std::char_traits<char>::move;

private:
    static char to_upper(char ch) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
};
