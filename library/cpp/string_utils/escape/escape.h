#pragma once

#include <string>

template <class TChar>
std::basic_string<TChar>& EscapeCImpl(const TChar* str, size_t len, std::basic_string<TChar>&);

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
static inline std::basic_string<TChar> EscapeC(TChar ch) {
    return EscapeC(&ch, 1);
}

template <typename TChar>
static inline std::basic_string<TChar> EscapeC(const TChar* str) {
    return EscapeC(str, std::char_traits<TChar>::length(str));
}

std::string& EscapeC(const std::string_view str, std::string& res);
std::u16string& EscapeC(const std::u16string_view str, std::u16string& res);

// these two need to be methods, because of TBasicString::Quote implementation
std::string EscapeC(const std::string& str);
std::u16string EscapeC(const std::u16string& str);

