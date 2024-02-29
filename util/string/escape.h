#pragma once

#include <util/generic/string.h>

template <class TChar>
TBasicString<TChar>& EscapeCImpl(const TChar* str, size_t len, TBasicString<TChar>&);

template <class TChar>
TBasicString<TChar>& UnescapeCImpl(const TChar* str, size_t len, TBasicString<TChar>&);

template <class TChar>
TChar* UnescapeC(const TChar* str, size_t len, TChar* buf);

template <typename TChar>
static inline TBasicString<TChar>& EscapeC(const TChar* str, size_t len, TBasicString<TChar>& s) {
    return EscapeCImpl(str, len, s);
}

template <typename TChar>
static inline TBasicString<TChar> EscapeC(const TChar* str, size_t len) {
    TBasicString<TChar> s;
    return EscapeC(str, len, s);
}

template <typename TChar>
static inline TBasicString<TChar> EscapeC(const std::basic_string_view<TChar>& str) {
    return EscapeC(str.data(), str.size());
}

template <typename TChar>
static inline TBasicString<TChar>& UnescapeC(const TChar* str, size_t len, TBasicString<TChar>& s) {
    return UnescapeCImpl(str, len, s);
}

template <typename TChar>
static inline TBasicString<TChar> UnescapeC(const TChar* str, size_t len) {
    TBasicString<TChar> s;
    return UnescapeCImpl(str, len, s);
}

template <typename TChar>
static inline TBasicString<TChar> EscapeC(TChar ch) {
    return EscapeC(&ch, 1);
}

template <typename TChar>
static inline TBasicString<TChar> EscapeC(const TChar* str) {
    return EscapeC(str, std::char_traits<TChar>::length(str));
}

std::string& EscapeC(const std::string_view str, std::string& res);
TUtf16String& EscapeC(const std::u16string_view str, TUtf16String& res);

// these two need to be methods, because of TBasicString::Quote implementation
std::string EscapeC(const std::string& str);
TUtf16String EscapeC(const TUtf16String& str);

std::string& UnescapeC(const std::string_view str, std::string& res);
TUtf16String& UnescapeC(const std::u16string_view str, TUtf16String& res);

std::string UnescapeC(const std::string_view str);
TUtf16String UnescapeC(const std::u16string_view wtr);

/// Returns number of chars in escape sequence.
///   - 0, if begin >= end
///   - 1, if [begin, end) starts with an unescaped char
///   - at least 2 (including '\'), if [begin, end) starts with an escaped symbol
template <class TChar>
size_t UnescapeCCharLen(const TChar* begin, const TChar* end);

namespace NQuote {
std::string Quote(std::string_view s);
}
