#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
=======
#include <src/util/generic/fwd.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/fwd.h>
>>>>>>> origin/main
#include <string_view>

namespace NUrl {

    /**
     * Splits URL to host and path
     * Example:
     * auto [host, path] = SplitUrlToHostAndPath(url);
     *
     * @param[in] url                   any URL
     * @param[out] <host, path>     parsed host and path
     */
    struct TSplitUrlToHostAndPathResult {
        std::string_view host;
        std::string_view path;
    };

    Y_PURE_FUNCTION
    TSplitUrlToHostAndPathResult SplitUrlToHostAndPath(const std::string_view url);

} // namespace NUrl

Y_PURE_FUNCTION
size_t GetHttpPrefixSize(const char* url, bool ignorehttps = false) noexcept;
Y_PURE_FUNCTION
size_t GetHttpPrefixSize(const wchar16* url, bool ignorehttps = false) noexcept;

Y_PURE_FUNCTION
size_t GetHttpPrefixSize(const std::string_view url, bool ignorehttps = false) noexcept;

Y_PURE_FUNCTION
size_t GetHttpPrefixSize(const std::u16string_view url, bool ignorehttps = false) noexcept;

/** BEWARE of std::string_view! You can not use operator ~ or c_str() like in std::string
    !!!!!!!!!!!! */
Y_PURE_FUNCTION
size_t GetSchemePrefixSize(const std::string_view url) noexcept;

Y_PURE_FUNCTION
std::string_view GetSchemePrefix(const std::string_view url) noexcept;

//! removes protocol prefixes 'http://' and 'https://' from given URL
//! @note if URL has no prefix or some other prefix the function does nothing
//! @param url    URL from which the prefix should be removed
//! @param ignorehttps if true, leaves https://
//! @return a new URL without protocol prefix
Y_PURE_FUNCTION
std::string_view CutHttpPrefix(const std::string_view url, bool ignorehttps = false) noexcept;

Y_PURE_FUNCTION
std::u16string_view CutHttpPrefix(const std::u16string_view url, bool ignorehttps = false) noexcept;

Y_PURE_FUNCTION
std::string_view CutSchemePrefix(const std::string_view url) noexcept;

//! adds specified scheme prefix if URL has no scheme
//! @note if URL has scheme prefix already the function returns unchanged URL
std::string AddSchemePrefix(const std::string& url, const std::string_view scheme);

//! Same as `AddSchemePrefix(url, "http")`.
std::string AddSchemePrefix(const std::string& url);

Y_PURE_FUNCTION
std::string_view GetHost(const std::string_view url) noexcept;

Y_PURE_FUNCTION
std::string_view GetHostAndPort(const std::string_view url) noexcept;

Y_PURE_FUNCTION
std::string_view GetSchemeHost(const std::string_view url, bool trimHttp = true) noexcept;

Y_PURE_FUNCTION
std::string_view GetSchemeHostAndPort(const std::string_view url, bool trimHttp = true, bool trimDefaultPort = true) noexcept;

/**
 * Splits URL to host and path
 *
 * @param[in] url       any URL
 * @param[out] host     parsed host
 * @param[out] path     parsed path
 */
void SplitUrlToHostAndPath(const std::string_view url, std::string_view& host, std::string_view& path);
void SplitUrlToHostAndPath(const std::string_view url, std::string& host, std::string& path);

/**
 * Separates URL into url prefix, query (aka cgi params list), and fragment (aka part after #)
 *
 * @param[in] url               any URL
 * @param[out] sanitizedUrl     parsed URL without query and fragment parts
 * @param[out] query            parsed query
 * @param[out] fragment         parsed fragment
 */
void SeparateUrlFromQueryAndFragment(const std::string_view url, std::string_view& sanitizedUrl, std::string_view& query, std::string_view& fragment);

/**
 * Extracts scheme, host and port from URL.
 *
 * Port will be parsed from URL with checks against ui16 overflow. If URL doesn't
 * contain port it will be determined by one of the known schemes (currently
 * https:// and http:// only).
 * Given parameters will not be modified if URL has no appropriate components.
 *
 * @param[in] url       any URL
 * @param[out] scheme   URL scheme
 * @param[out] host     host name
 * @param[out] port     parsed port number
 * @return false if present port number cannot be parsed into ui16
 *         true  otherwise.
 */
bool TryGetSchemeHostAndPort(const std::string_view url, std::string_view& scheme, std::string_view& host, ui16& port);

/**
 * Extracts scheme, host and port from URL.
 *
 * This function perform the same actions as TryGetSchemeHostAndPort(), but in
 * case of impossibility to parse port number throws yexception.
 *
 * @param[in] url       any URL
 * @param[out] scheme   URL scheme
 * @param[out] host     host name
 * @param[out] port     parsed port number
 * @throws yexception  if present port number cannot be parsed into ui16.
 */
void GetSchemeHostAndPort(const std::string_view url, std::string_view& scheme, std::string_view& host, ui16& port);

Y_PURE_FUNCTION
std::string_view GetPathAndQuery(const std::string_view url, bool trimFragment = true) noexcept;
/**
 * Extracts host from url and cuts http(https) protocol prefix and port if any.
 * @param[in] url   any URL
 * @return          host without port and http(https) prefix.
 */
Y_PURE_FUNCTION
std::string_view GetOnlyHost(const std::string_view url) noexcept;

Y_PURE_FUNCTION
std::string_view GetParentDomain(const std::string_view host, size_t level) noexcept; // ("www.ya.ru", 2) -> "ya.ru"

Y_PURE_FUNCTION
std::string_view GetZone(const std::string_view host) noexcept;

Y_PURE_FUNCTION
std::string_view CutWWWPrefix(const std::string_view url) noexcept;

Y_PURE_FUNCTION
std::string_view CutWWWNumberedPrefix(const std::string_view url) noexcept;

/**
 * Cuts 'm.' prefix from url if and only if the url starts with it
 * Example: 'm.some-domain.com' -> 'some-domain.com'.
 * 'http://m.some-domain.com' is not changed
 *
 * @param[in] url   any URL
 * @return          url without 'm.' or 'M.' prefix.
 */
Y_PURE_FUNCTION
std::string_view CutMPrefix(const std::string_view url) noexcept;

Y_PURE_FUNCTION
std::string_view GetDomain(const std::string_view host) noexcept; // should not be used

size_t NormalizeUrlName(char* dest, const std::string_view source, size_t dest_size);
size_t NormalizeHostName(char* dest, const std::string_view source, size_t dest_size, ui16 defport = 80);

Y_PURE_FUNCTION
std::string_view RemoveFinalSlash(std::string_view str) noexcept;

std::string_view CutUrlPrefixes(std::string_view url) noexcept;
bool DoesUrlPathStartWithToken(std::string_view url, const std::string_view& token) noexcept;

