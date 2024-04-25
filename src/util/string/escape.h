#pragma once

<<<<<<<< HEAD:src/util/string/escape.h
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/string/escape.h
#include <ydb-cpp-sdk/library/string_utils/helpers/helpers.h>
========
#include <src/library/string_utils/helpers/helpers.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/string/escape.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/string/escape.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
========
#include <ydb-cpp-sdk/library/string_utils/helpers/helpers.h>
>>>>>>>> origin/main:include/ydb-cpp-sdk/util/string/escape.h

template <class TChar>
std::basic_string<TChar>& EscapeCImpl(const TChar* str, size_t len, std::basic_string<TChar>&);

template <class TChar>
std::basic_string<TChar>& UnescapeCImpl(const TChar* str, size_t len, std::basic_string<TChar>&);

template <class TChar>
TChar* UnescapeC(const TChar* str, size_t len, TChar* buf);

template <typename TChar>
static inline std::basic_string<TChar>& EscapeC(const TChar* str, size_t len, std::basic_string<TChar>& s) {
    return EscapeCImpl(str, len, s);
}

template <typename TChar>
static inline std::basic_string<TChar> EscapeC(const TChar* str, size_t len) {
    std::basic_string<TChar> s;
    return EscapeC(str, len, s);
}

template <typename TChar>
static inline std::basic_string<TChar> EscapeC(const std::basic_string_view<TChar>& str) {
    return EscapeC(str.data(), str.size());
}

template <typename TChar>
static inline std::basic_string<TChar>& UnescapeC(const TChar* str, size_t len, std::basic_string<TChar>& s) {
    return UnescapeCImpl(str, len, s);
}

template <typename TChar>
static inline std::basic_string<TChar> UnescapeC(const TChar* str, size_t len) {
    std::basic_string<TChar> s;
    return UnescapeCImpl(str, len, s);
}

template <typename TChar>
static inline std::basic_string<TChar> EscapeC(TChar ch) {
    return EscapeC(&ch, 1);
}

template <typename TChar>
static inline std::basic_string<TChar> EscapeC(const TChar* str) {
    return EscapeC(str, std::char_traits<TChar>::length(str));
}

std::string& EscapeC(const std::string_view str, std::string& res);
std::u16string& EscapeC(const std::u16string_view str, std::u16string& res);

// these two need to be methods, because of std::basic_string::Quote implementation
std::string EscapeC(const std::string& str);
std::u16string EscapeC(const std::u16string& str);

std::string& UnescapeC(const std::string_view str, std::string& res);
std::u16string& UnescapeC(const std::u16string_view str, std::u16string& res);

std::string UnescapeC(const std::string_view str);
std::u16string UnescapeC(const std::u16string_view wtr);

/// Returns number of chars in escape sequence.
///   - 0, if begin >= end
///   - 1, if [begin, end) starts with an unescaped char
///   - at least 2 (including '\'), if [begin, end) starts with an escaped symbol
template <class TChar>
size_t UnescapeCCharLen(const TChar* begin, const TChar* end);

namespace NUtils {

template <class TChar>
std::basic_string<TChar> GetQuoteLiteral();

template <class TChar>
std::basic_string<TChar> Quote(std::basic_string_view<TChar> s) {
    return GetQuoteLiteral<TChar>() + EscapeC(s) + GetQuoteLiteral<TChar>();
}

template <class TChar>
std::basic_string<TChar> Quote(const std::basic_string<TChar>& s) {
    return Quote(std::basic_string_view<TChar>(s));
}

}
