#pragma once

#include <ydb-cpp-sdk/util/system/compat.h>

#include <cctype>
#include <string>

struct TCaseInsensitiveCharTraits : private std::char_traits<char> {
private:
    static constexpr bool IsNothrowToUpper = noexcept(std::toupper(0));

    using TBaseTraits = std::char_traits<char>;

public:
    using TBaseTraits::char_type;
    using TBaseTraits::int_type;
    using TBaseTraits::off_type;
    using TBaseTraits::pos_type;
    using TBaseTraits::state_type;

public:
    static bool eq(char_type c1, char_type c2) noexcept(IsNothrowToUpper) {
        return to_upper(c1) == to_upper(c2);
    }

    static bool lt(char_type c1, char_type c2) noexcept(IsNothrowToUpper) {
        return to_upper(c1) < to_upper(c2);
    }

    static bool eq_int_type(int_type c1, int_type c2) noexcept(IsNothrowToUpper) {
        return eq(to_char_type(c1), to_char_type(c2));
    }

    static int compare(const char_type* s1, const char_type* s2, std::size_t n)
            noexcept(IsNothrowToUpper) {
        return strnicmp(s1, s2, n);
    }

    static const char_type* find(const char_type* s, std::size_t n, char_type a)
            noexcept(IsNothrowToUpper);

    using TBaseTraits::length;
    using TBaseTraits::move;
    using TBaseTraits::copy;
    using TBaseTraits::assign;
    using TBaseTraits::not_eof;
    using TBaseTraits::to_char_type;
    using TBaseTraits::to_int_type;
    using TBaseTraits::eof;

private:
    static char_type to_upper(char_type ch) noexcept(IsNothrowToUpper) {
        return static_cast<char_type>(std::toupper(static_cast<unsigned char>(ch)));
    }
};
