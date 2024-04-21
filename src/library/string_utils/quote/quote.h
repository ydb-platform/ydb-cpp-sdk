#pragma once

#include <string>

//CGIEscape*:
// ' ' converted to '+',
// Some punctuation and chars outside [32, 126] range are converted to %xx
// Use function CgiEscapeBufLen to determine number of characters needed for 'char* to' parameter.
// Returns pointer to the end of the result string
char* CGIEscape(char* to, const char* from);
char* CGIEscape(char* to, const char* from, size_t len);
inline char* CGIEscape(char* to, const std::string_view from) {
    return CGIEscape(to, from.data(), from.size());
}
void CGIEscape(std::string& url);
std::string CGIEscapeRet(const std::string_view url);
std::string& AppendCgiEscaped(const std::string_view value, std::string& to);

inline std::string_view CgiEscapeBuf(char* to, const std::string_view from) {
    return std::string_view(to, CGIEscape(to, from.data(), from.size()));
}
inline std::string_view CgiEscape(void* tmp, const std::string_view s) {
    return CgiEscapeBuf(static_cast<char*>(tmp), s);
}

//CgiUnescape*:
// Decodes '%xx' to bytes, '+' to space.
// Use function CgiUnescapeBufLen to determine number of characters needed for 'char* to' parameter.
// If pointer returned, then this is pointer to the end of the result string.
char* CGIUnescape(char* to, const char* from);
char* CGIUnescape(char* to, const char* from, size_t len);
void CGIUnescape(std::string& url);
std::string CGIUnescapeRet(const std::string_view from);

inline std::string_view CgiUnescapeBuf(char* to, const std::string_view from) {
    return std::string_view(to, CGIUnescape(to, from.data(), from.size()));
}
inline std::string_view CgiUnescape(void* tmp, const std::string_view s) {
    return CgiUnescapeBuf(static_cast<char*>(tmp), s);
}

//Quote:
// Is like CGIEscape, also skips encoding of user-supplied 'safe' characters.
char* Quote(char* to, const char* from, const char* safe = "/");
char* Quote(char* to, const std::string_view s, const char* safe = "/");
void Quote(std::string& url, const char* safe = "/");

//UrlEscape:
// Can't be used for cgi parameters ('&' character is not escaped)!
// escapes only '%' not followed by two hex-digits or if forceEscape set to ture,
// and chars outside [32, 126] range.
// Can't handle '\0'-chars in std::string.
char* UrlEscape(char* to, const char* from, bool forceEscape = false);
void UrlEscape(std::string& url, bool forceEscape = false);
std::string UrlEscapeRet(const std::string_view from, bool forceEscape = false);

//UrlUnescape:
// '+' is NOT converted to space!
// %xx converted to bytes, other characters are copied unchanged.
char* UrlUnescape(char* to, std::string_view from);
void UrlUnescape(std::string& url);
std::string UrlUnescapeRet(const std::string_view from);

//*BufLen: how much characters you should allocate for 'char* to' buffers.
constexpr size_t CgiEscapeBufLen(const size_t len) noexcept {
    return 3 * len + 1;
}

constexpr size_t CgiUnescapeBufLen(const size_t len) noexcept {
    return len + 1;
}
