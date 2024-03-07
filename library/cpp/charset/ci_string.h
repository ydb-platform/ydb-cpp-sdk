#pragma once

#include "codepage.h"

#include <string>
#include <util/str_stl.h>

// Same as std::string but uses CASE INSENSITIVE comparator and hash. Use with care.
class TCiString: public std::string {
public:
    TCiString() {
    }

    TCiString(const std::string& s)
        : std::string(s)
    {
    }

    TCiString(const std::string& s, size_t pos, size_t n)
        : std::string(s, pos, n)
    {
    }

    TCiString(const char* pc)
        : std::string(pc)
    {
    }

    TCiString(const char* pc, size_t n)
        : std::string(pc, n)
    {
    }

    TCiString(const char* pc, size_t pos, size_t n)
        : std::string(pc, pos, n)
    {
    }

    TCiString(size_t n, char c)
        : std::string(n, c)
    {
    }

    TCiString(const char* b, const char* e)
        : std::string(b, e)
    {
    }

    explicit TCiString(const std::string_view& s)
        : std::string(s)
    {
    }

    // ~~~ Comparison ~~~ : FAMILY0(int, compare)
    static int compare(const TCiString& s1, const TCiString& s2, const CodePage& cp = csYandex);
    static int compare(const char* p, const TCiString& s2, const CodePage& cp = csYandex);
    static int compare(const TCiString& s1, const char* p, const CodePage& cp = csYandex);
    static int compare(const std::string_view& p1, const std::string_view& p2, const CodePage& cp = csYandex);

    // TODO: implement properly in std::string via enum ECaseSensitivity
    static bool is_prefix(const std::string_view& what, const std::string_view& of, const CodePage& cp = csYandex);
    static bool is_suffix(const std::string_view& what, const std::string_view& of, const CodePage& cp = csYandex);

    bool StartsWith(const std::string_view& s, const CodePage& cp = csYandex) const {
        return is_prefix(s, *this, cp);
    }

    bool EndsWith(const std::string_view& s, const CodePage& cp = csYandex) const {
        return is_suffix(s, *this, cp);
    }

    friend bool operator==(const TCiString& s1, const TCiString& s2) {
        return TCiString::compare(s1, s2) == 0;
    }

    friend bool operator==(const TCiString& s, const char* pc) {
        return TCiString::compare(s, pc) == 0;
    }

    friend bool operator==(const char* pc, const TCiString& s) {
        return TCiString::compare(pc, s) == 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator==(const TCiString& s, const std::basic_string<TDerived2, TChar, TTraits2>& pc) {
        return TCiString::compare(s, pc) == 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator==(const std::basic_string<TDerived2, TChar, TTraits2>& pc, const TCiString& s) {
        return TCiString::compare(pc, s) == 0;
    }

    friend bool operator!=(const TCiString& s1, const TCiString& s2) {
        return TCiString::compare(s1, s2) != 0;
    }

    friend bool operator!=(const TCiString& s, const char* pc) {
        return TCiString::compare(s, pc) != 0;
    }

    friend bool operator!=(const char* pc, const TCiString& s) {
        return TCiString::compare(pc, s) != 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator!=(const TCiString& s, const std::basic_string<TDerived2, TChar, TTraits2>& pc) {
        return TCiString::compare(s, pc) != 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator!=(const std::basic_string<TDerived2, TChar, TTraits2>& pc, const TCiString& s) {
        return TCiString::compare(pc, s) != 0;
    }

    friend bool operator<(const TCiString& s1, const TCiString& s2) {
        return TCiString::compare(s1, s2) < 0;
    }

    friend bool operator<(const TCiString& s, const char* pc) {
        return TCiString::compare(s, pc) < 0;
    }

    friend bool operator<(const char* pc, const TCiString& s) {
        return TCiString::compare(pc, s) < 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator<(const TCiString& s, const std::basic_string<TDerived2, TChar, TTraits2>& pc) {
        return TCiString::compare(s, pc) < 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator<(const std::basic_string<TDerived2, TChar, TTraits2>& pc, const TCiString& s) {
        return TCiString::compare(pc, s) < 0;
    }

    friend bool operator<=(const TCiString& s1, const TCiString& s2) {
        return TCiString::compare(s1, s2) <= 0;
    }

    friend bool operator<=(const TCiString& s, const char* pc) {
        return TCiString::compare(s, pc) <= 0;
    }

    friend bool operator<=(const char* pc, const TCiString& s) {
        return TCiString::compare(pc, s) <= 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator<=(const TCiString& s, const std::basic_string<TDerived2, TChar, TTraits2>& pc) {
        return TCiString::compare(s, pc) <= 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator<=(const std::basic_string<TDerived2, TChar, TTraits2>& pc, const TCiString& s) {
        return TCiString::compare(pc, s) <= 0;
    }

    friend bool operator>(const TCiString& s1, const TCiString& s2) {
        return TCiString::compare(s1, s2) > 0;
    }

    friend bool operator>(const TCiString& s, const char* pc) {
        return TCiString::compare(s, pc) > 0;
    }

    friend bool operator>(const char* pc, const TCiString& s) {
        return TCiString::compare(pc, s) > 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator>(const TCiString& s, const std::basic_string<TDerived2, TChar, TTraits2>& pc) noexcept {
        return TCiString::compare(s, pc) > 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator>(const std::basic_string<TDerived2, TChar, TTraits2>& pc, const TCiString& s) noexcept {
        return TCiString::compare(pc, s) > 0;
    }

    friend bool operator>=(const TCiString& s1, const TCiString& s2) {
        return TCiString::compare(s1, s2) >= 0;
    }

    friend bool operator>=(const TCiString& s, const char* pc) {
        return TCiString::compare(s, pc) >= 0;
    }

    friend bool operator>=(const char* pc, const TCiString& s) {
        return TCiString::compare(pc, s) >= 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator>=(const TCiString& s, const std::basic_string<TDerived2, TChar, TTraits2>& pc) {
        return TCiString::compare(s, pc) >= 0;
    }

    template <typename TDerived2, typename TTraits2>
    friend bool operator>=(const std::basic_string<TDerived2, TChar, TTraits2>& pc, const TCiString& s) {
        return TCiString::compare(pc, s) >= 0;
    }

    static size_t hashVal(const char* pc, size_t len, const CodePage& cp = csYandex);

    size_t hash() const {
        return TCiString::hashVal(data(), length());
    }
};

struct ci_hash {
    inline size_t operator()(const char* s) const {
        return TCiString::hashVal(s, strlen(s));
    }
    inline size_t operator()(const std::string_view& s) const {
        return TCiString::hashVal(s.data(), s.size());
    }
};

struct ci_hash32 { // not the same as ci_hash under 64-bit
    inline ui32 operator()(const char* s) const {
        return (ui32)TCiString::hashVal(s, strlen(s));
    }
};

//template <class T> struct hash;

template <>
struct hash<TCiString>: public ci_hash {
};

template <class T>
struct TCIHash {
};

template <>
struct TCIHash<const char*> {
    inline size_t operator()(const std::string_view& s) const {
        return TCiString::hashVal(s.data(), s.size());
    }
};

template <>
struct TCIHash<std::string_view> {
    inline size_t operator()(const std::string_view& s) const {
        return TCiString::hashVal(s.data(), s.size());
    }
};

template <>
struct TCIHash<std::string> {
    inline size_t operator()(const std::string& s) const {
        return TCiString::hashVal(s.data(), s.size());
    }
};

struct ci_less {
    inline bool operator()(const char* x, const char* y) const {
        return csYandex.Stricmp(x, y) < 0;
    }
};

struct ci_equal_to {
    inline bool operator()(const char* x, const char* y) const {
        return csYandex.Stricmp(x, y) == 0;
    }
    // this implementation is not suitable for strings with zero characters inside, sorry
    bool operator()(const std::string_view& x, const std::string_view& y) const {
        return x.size() == y.size() && csYandex.Strnicmp(x.data(), y.data(), y.size()) == 0;
    }
};

template <>
struct TEqualTo<TCiString>: public ci_equal_to {
};
