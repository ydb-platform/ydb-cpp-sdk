#include "url.h"

#include <src/util/charset/unidata.h> // for ToLower
#include <src/util/string/cstriter.h>
#include <src/util/string/util.h>

#include <ydb-cpp-sdk/library/string_utils/helpers/helpers.h>
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

#include <ydb-cpp-sdk/util/generic/algorithm.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>

#include <ydb-cpp-sdk/util/string/builder.h>
#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/string/ascii.h>
#include <ydb-cpp-sdk/util/string/strip.h>

#include <ydb-cpp-sdk/util/system/defaults.h>

#include <cstdlib>

namespace {
    struct TUncheckedSize {
        static bool Has(size_t) {
            return true;
        }
    };

    struct TKnownSize {
        size_t MySize;
        explicit TKnownSize(size_t sz)
            : MySize(sz)
        {
        }
        bool Has(size_t sz) const {
            return sz <= MySize;
        }
    };

    template <typename TChar1, typename TChar2>
    int Compare1Case2(const TChar1* s1, const TChar2* s2, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            if (static_cast<TChar1>(ToLower(s1[i])) != s2[i]) {
                return static_cast<TChar1>(ToLower(s1[i])) < s2[i] ? -1 : 1;
            }
        }
        return 0;
    }

    template <typename TChar, typename TBounds>
    inline size_t GetHttpPrefixSizeImpl(const TChar* url, const TBounds& urlSize, bool ignorehttps) {
        const TChar httpPrefix[] = {'h', 't', 't', 'p', ':', '/', '/', 0};
        const TChar httpsPrefix[] = {'h', 't', 't', 'p', 's', ':', '/', '/', 0};
        if (urlSize.Has(7) && Compare1Case2(url, httpPrefix, 7) == 0) {
            return 7;
        }
        if (!ignorehttps && urlSize.Has(8) && Compare1Case2(url, httpsPrefix, 8) == 0) {
            return 8;
        }
        return 0;
    }

    template <typename T>
    inline T CutHttpPrefixImpl(const T& url, bool ignorehttps) {
        size_t prefixSize = GetHttpPrefixSizeImpl<typename T::value_type>(
                url.data(), TKnownSize(url.size()), ignorehttps);
        if (prefixSize) {
            return url.substr(prefixSize);
        }
        return url;
    }
}

namespace NUrl {

    TSplitUrlToHostAndPathResult SplitUrlToHostAndPath(const std::string_view url) {
        std::string_view host = GetSchemeHostAndPort(url, /*trimHttp=*/false, /*trimDefaultPort=*/false);
        std::string_view path = url;
        if (path.starts_with(host)) {
            path.remove_prefix(host.size());
        }
        return {host, path};
    }

} // namespace NUrl

size_t GetHttpPrefixSize(const char* url, bool ignorehttps) noexcept {
    return GetHttpPrefixSizeImpl<char>(url, TUncheckedSize(), ignorehttps);
}

size_t GetHttpPrefixSize(const wchar16* url, bool ignorehttps) noexcept {
    return GetHttpPrefixSizeImpl<wchar16>(url, TUncheckedSize(), ignorehttps);
}

size_t GetHttpPrefixSize(const std::string_view url, bool ignorehttps) noexcept {
    return GetHttpPrefixSizeImpl<char>(url.data(), TKnownSize(url.size()), ignorehttps);
}

size_t GetHttpPrefixSize(const std::u16string_view url, bool ignorehttps) noexcept {
    return GetHttpPrefixSizeImpl<wchar16>(url.data(), TKnownSize(url.size()), ignorehttps);
}

std::string_view CutHttpPrefix(const std::string_view url, bool ignorehttps) noexcept {
    return CutHttpPrefixImpl(url, ignorehttps);
}

std::u16string_view CutHttpPrefix(const std::u16string_view url, bool ignorehttps) noexcept {
    return CutHttpPrefixImpl(url, ignorehttps);
}

size_t GetSchemePrefixSize(const std::string_view url) noexcept {
    if (url.empty()) {
        return 0;
    }

    struct TDelim: public str_spn {
        inline TDelim()
            : str_spn("!-/:-@[-`{|}", true)
        {
        }
    };

    const auto& delim = *Singleton<TDelim>();
    const char* n = delim.brk(url.data(), url.end());

    if (n + 2 >= url.end() || *n != ':' || n[1] != '/' || n[2] != '/') {
        return 0;
    }

    return n + 3 - url.begin();
}

std::string_view GetSchemePrefix(const std::string_view url) noexcept {
    return url.substr(0, GetSchemePrefixSize(url));
}

std::string_view CutSchemePrefix(const std::string_view url) noexcept {
    return url.substr(GetSchemePrefixSize(url));
}

template <bool KeepPort>
static inline std::string_view GetHostAndPortImpl(const std::string_view url) {
    std::string_view urlNoScheme = url;

    urlNoScheme.remove_prefix(GetHttpPrefixSize(url));

    struct TDelim: public str_spn {
        inline TDelim()
            : str_spn(KeepPort ? "/;?#" : "/:;?#")
        {
        }
    };

    const auto& nonHostCharacters = *Singleton<TDelim>();
    const char* firstNonHostCharacter = nonHostCharacters.brk(urlNoScheme.begin(), urlNoScheme.end());

    if (firstNonHostCharacter != urlNoScheme.end()) {
        return urlNoScheme.substr(0, firstNonHostCharacter - urlNoScheme.data());
    }

    return urlNoScheme;
}

std::string_view GetHost(const std::string_view url) noexcept {
    return GetHostAndPortImpl<false>(url);
}

std::string_view GetHostAndPort(const std::string_view url) noexcept {
    return GetHostAndPortImpl<true>(url);
}

std::string_view GetSchemeHost(const std::string_view url, bool trimHttp) noexcept {
    const size_t schemeSize = GetSchemePrefixSize(url);
    const std::string_view scheme = url.substr(0, schemeSize);

    const bool isHttp = (schemeSize == 0 || scheme == std::string_view("http://"));

    const std::string_view host = GetHost(url.substr(schemeSize));

    if (isHttp && trimHttp) {
        return host;
    } else {
        return std::string_view(scheme.begin(), host.end());
    }
}

std::string_view GetSchemeHostAndPort(const std::string_view url, bool trimHttp, bool trimDefaultPort) noexcept {
    const size_t schemeSize = GetSchemePrefixSize(url);
    const std::string_view scheme = url.substr(0, schemeSize);

    const bool isHttp = (schemeSize == 0 || scheme == std::string_view("http://"));

    std::string_view hostAndPort = GetHostAndPort(url.substr(schemeSize));

    if (trimDefaultPort) {
        const auto pos = hostAndPort.find(':');
        if (pos != std::string_view::npos) {
            const bool isHttps = (scheme == std::string_view("https://"));

            const std::string_view port = hostAndPort.substr(pos + 1);
            if ((isHttp && port == std::string_view("80")) || (isHttps && port == std::string_view("443"))) {
                // trimming default port
                hostAndPort = hostAndPort.substr(0, pos);
            }
        }
    }

    if (isHttp && trimHttp) {
        return hostAndPort;
    } else {
        return std::string_view(scheme.begin(), hostAndPort.end());
    }
}

void SplitUrlToHostAndPath(const std::string_view url, std::string_view& host, std::string_view& path) {
    auto [hostBuf, pathBuf] = NUrl::SplitUrlToHostAndPath(url);
    host = hostBuf;
    path = pathBuf;
}

void SplitUrlToHostAndPath(const std::string_view url, std::string& host, std::string& path) {
    auto [hostBuf, pathBuf] = NUrl::SplitUrlToHostAndPath(url);
    host = hostBuf;
    path = pathBuf;
}

void SeparateUrlFromQueryAndFragment(const std::string_view url, std::string_view& sanitizedUrl, std::string_view& query, std::string_view& fragment) {
    std::string_view urlWithoutFragment;
    NUtils::Split(url, urlWithoutFragment, fragment, '#');
    NUtils::Split(urlWithoutFragment, sanitizedUrl, query, '?');
}

bool TryGetSchemeHostAndPort(const std::string_view url, std::string_view& scheme, std::string_view& host, ui16& port) {
    const size_t schemeSize = GetSchemePrefixSize(url);
    if (schemeSize != 0) {
        scheme = url.substr(0, schemeSize);
    }

    std::string_view portStr;
    std::string_view hostAndPort = GetHostAndPort(url.substr(schemeSize));
    if (!hostAndPort.empty() && hostAndPort.back() != ']' && NUtils::TryRSplit(hostAndPort, host, portStr, ':')) {
        // URL has port
        if (!TryFromString(portStr, port)) {
            return false;
        }
    } else {
        host = hostAndPort;
        if (scheme == std::string_view("https://")) {
            port = 443;
        } else if (scheme == std::string_view("http://")) {
            port = 80;
        }
    }
    return true;
}

void GetSchemeHostAndPort(const std::string_view url, std::string_view& scheme, std::string_view& host, ui16& port) {
    bool isOk = TryGetSchemeHostAndPort(url, scheme, host, port);
    Y_ENSURE(isOk, "cannot parse port number from URL: " << url);
}

std::string_view GetOnlyHost(const std::string_view url) noexcept {
    return GetHost(CutSchemePrefix(url));
}

std::string_view GetPathAndQuery(const std::string_view url, bool trimFragment) noexcept {
    const size_t off = url.find('/', GetHttpPrefixSize(url));
    std::string_view hostUnused, path;
    if (!NUtils::TrySplitOn(url, hostUnused, path, off, 0)) {
        return "/";
    }

    return trimFragment ? NUtils::Before(path, '#') : path;
}

// this strange creature returns 2nd level domain, possibly with port
std::string_view GetDomain(const std::string_view host) noexcept {
    const char* c = host.empty() ? host.data() : host.end() - 1;
    for (bool wasPoint = false; c != host.data(); --c) {
        if (*c == '.') {
            if (wasPoint) {
                ++c;
                break;
            }
            wasPoint = true;
        }
    }
    return std::string_view(c, host.end());
}

std::string_view GetParentDomain(const std::string_view host, size_t level) noexcept {
    auto pos = host.size();
    for (size_t i = 0; i < level; ++i) {
        pos = host.rfind('.', pos);
        if (pos == std::string::npos) {
            return host;
        }
    }
    return host.substr(pos + 1);
}

std::string_view GetZone(const std::string_view host) noexcept {
    return GetParentDomain(host, 1);
}

std::string_view CutWWWPrefix(const std::string_view url) noexcept {
    if (url.size() >= 4 && url[3] == '.' && !strnicmp(url.data(), "www", 3)) {
        return url.substr(4);
    }
    return url;
}

std::string_view CutWWWNumberedPrefix(const std::string_view url) noexcept {
    auto it = url.begin();

    StripRangeBegin(it, url.end(), [](auto& it){ return *it == 'w' || *it == 'W'; });
    if (it == url.begin()) {
        return url;
    }

    StripRangeBegin(it, url.end(), [](auto& it){ return IsAsciiDigit(*it); });
    if (it == url.end()) {
        return url;
    }

    if (*it++ == '.') {
        return url.substr(it - url.begin());
    }

    return url;
}

std::string_view CutMPrefix(const std::string_view url) noexcept {
    if (url.size() >= 2 && url[1] == '.' && (url[0] == 'm' || url[0] == 'M')) {
        return url.substr(2);
    }
    return url;
}

static inline bool IsSchemeChar(char c) noexcept {
    return IsAsciiAlnum(c); //what about '+' ?..
}

static bool HasPrefix(const std::string_view url) noexcept {
    std::string_view scheme, unused;
    if (!NUtils::TrySplit(url, scheme, unused, std::string_view("://"))) {
        return false;
    }

    return AllOf(scheme, IsSchemeChar);
}

std::string AddSchemePrefix(const std::string& url) {
    return AddSchemePrefix(url, std::string_view("http"));
}

std::string AddSchemePrefix(const std::string& url, std::string_view scheme) {
    if (HasPrefix(url)) {
        return url;
    }

    return TStringBuilder() << scheme << std::string_view("://") << url;
}

#define X(c) (c >= 'A' ? ((c & 0xdf) - 'A') + 10 : (c - '0'))

static inline int x2c(unsigned char* x) {
    if (!IsAsciiHex(x[0]) || !IsAsciiHex(x[1])) {
        return -1;
    }
    return X(x[0]) * 16 + X(x[1]);
}

#undef X

static inline int Unescape(char* str) {
    char *to, *from;
    int dlen = 0;
    if ((str = strchr(str, '%')) == nullptr) {
        return dlen;
    }
    for (to = str, from = str; *from; from++, to++) {
        if ((*to = *from) == '%') {
            // SAFE: cast char* -> unsigned char* (see [basic.lval] par.11.3)
            int c = x2c(reinterpret_cast<unsigned char*>(from) + 1);
            *to = char((c > 0) ? c : '0');
            from += 2;
            dlen += 2;
        }
    }
    *to = 0; /* terminate it at the new length */
    return dlen;
}

size_t NormalizeUrlName(char* dest, const std::string_view source, size_t dest_size) {
    if (source.empty() || source[0] == '?') {
        return NUtils::Strlcpy(dest, "/", dest_size);
    }
    size_t len = Min(dest_size - 1, source.length());
    memcpy(dest, source.data(), len);
    dest[len] = 0;
    len -= Unescape(dest);
    NUtils::ToLower(dest);
    return len;
}

size_t NormalizeHostName(char* dest, const std::string_view source, size_t dest_size, ui16 defport) {
    size_t len = Min(dest_size - 1, source.length());
    memcpy(dest, source.data(), len);
    dest[len] = 0;
    char buf[8] = ":";
    size_t buflen = 1 + ToString(defport, buf + 1, sizeof(buf) - 2);
    buf[buflen] = '\0';
    char* ptr = strstr(dest, buf);
    if (ptr && ptr[buflen] == 0) {
        len -= buflen;
        *ptr = 0;
    }
    NUtils::ToLower(dest);
    return len;
}

std::string_view RemoveFinalSlash(std::string_view str) noexcept {
    if (str.ends_with('/')) {
        str.remove_suffix(1);
    }
    return str;
}

std::string_view CutUrlPrefixes(std::string_view url) noexcept {
    url = CutSchemePrefix(url);
    url = CutWWWPrefix(url);
    return url;
}

bool DoesUrlPathStartWithToken(std::string_view url, const std::string_view& token) noexcept {
    url = CutSchemePrefix(url);
    const std::string_view noHostSuffix = NUtils::After(url, '/');
    if (noHostSuffix == url) {
        // no slash => no suffix with token info
        return false;
    }
    const bool suffixHasPrefix = noHostSuffix.starts_with(token);
    if (!suffixHasPrefix) {
        return false;
    }
    const bool slashAfterPrefix = noHostSuffix.find("/", token.length()) == token.length();
    const bool qMarkAfterPrefix = noHostSuffix.find("?", token.length()) == token.length();
    const bool nothingAfterPrefix = noHostSuffix.length() <= token.length();
    const bool prefixIsToken = slashAfterPrefix || qMarkAfterPrefix || nothingAfterPrefix;
    return prefixIsToken;
}
