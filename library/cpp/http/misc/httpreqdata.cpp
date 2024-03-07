#include "httpreqdata.h"

#include <library/cpp/case_insensitive_string/case_insensitive_string.h>
#include <util/string/builder.h>
#include <library/cpp/string_utils/misc/misc.h>

#include <util/stream/mem.h>
#include <util/string/join.h>

#include <array>

#ifdef _sse4_2_
#include <smmintrin.h>
#endif

TBaseServerRequestData::TBaseServerRequestData(SOCKET s)
    : Socket_(s)
    , BeginTime_(MicroSeconds())
{
}

TBaseServerRequestData::TBaseServerRequestData(std::string_view qs, SOCKET s)
    : Query_(qs)
    , OrigQuery_(Query_)
    , Socket_(s)
    , BeginTime_(MicroSeconds())
{
}

void TBaseServerRequestData::AppendQueryString(std::string_view str) {
    using namespace std::literals;
    if (Y_UNLIKELY(!Query_.empty())) {
        std::string_view separator = !Query_.ends_with('&') && !str.starts_with('&') ? "&"sv : ""sv;
        ModifiedQueryString_ = TStringBuilder() << Query_ << separator << str;
     } else {
        ModifiedQueryString_ = str;
     }
    Query_ = ModifiedQueryString_;
}

void TBaseServerRequestData::SetRemoteAddr(std::string_view addr) {
    Addr_.emplace(addr.substr(0, INET6_ADDRSTRLEN - 1));
}

std::string_view TBaseServerRequestData::RemoteAddr() const {
    if (!Addr_) {
        auto& addr = Addr_.emplace();
        addr.resize(INET6_ADDRSTRLEN);
        if (GetRemoteAddr(Socket_, addr.begin(), addr.size())) {
            if (auto pos = addr.find('\0'); pos != std::string::npos) {
                addr.resize(pos);
            }
        } else {
            addr.clear();
        }
     }

    return *Addr_;
 }

const std::string* TBaseServerRequestData::HeaderIn(std::string_view key) const {
    return HeadersIn_.FindPtr(key);
}

std::string_view TBaseServerRequestData::HeaderInOrEmpty(std::string_view key) const {
    const auto* ptr = HeaderIn(key);
    return ptr ? std::string_view{*ptr} : std::string_view{};
}

std::string TBaseServerRequestData::HeaderByIndex(size_t n) const noexcept {
    if (n >= HeadersIn_.size()) {
        return {};
    }

    const auto& [key, value] = *std::next(HeadersIn_.begin(), n);

    return TStringBuilder() << key << ": " << value;
}

std::string_view TBaseServerRequestData::Environment(std::string_view key) const {
    TCaseInsensitiveStringBuf ciKey(key.data(), key.size());
    if (ciKey == "REMOTE_ADDR") {
        const auto ip = HeaderIn("X-Real-IP");
        return ip ? *ip : RemoteAddr();
    } else if (ciKey == "QUERY_STRING") {
        return Query();
    } else if (ciKey == "SERVER_NAME") {
        return ServerName();
    } else if (ciKey == "SERVER_PORT") {
        return ServerPort();
    } else if (ciKey == "SCRIPT_NAME") {
        return ScriptName();
    }
    return {};
}

 void TBaseServerRequestData::Clear() {
    HeadersIn_.clear();
    Addr_ = std::nullopt;
    Path_.clear();
    Query_ = {};
    OrigQuery_ = {};
    Host_.clear();
    Port_.clear();
    CurPage_.erase();
    ParseBuf_.clear();
    BeginTime_ = MicroSeconds();
}

const std::string& TBaseServerRequestData::GetCurPage() const {
    using namespace std::literals;
    if (CurPage_.empty() && !Host_.empty()) {
        std::array<std::string_view, 7> fragments;
        auto fragmentIt = fragments.begin();
        *fragmentIt++ = "http://"sv;
        *fragmentIt++ = Host_;
        if (!Port_.empty()) {
            *fragmentIt++ = ":"sv;
            *fragmentIt++ = Port_;
        }
        *fragmentIt++ = Path_;
        if (!Query_.empty()) {
            *fragmentIt++ = "?"sv;
            *fragmentIt++ = Query_;
        }

        CurPage_ = JoinRange(""sv, fragments.begin(), fragmentIt);
    }
    return CurPage_;
}

bool TBaseServerRequestData::Parse(std::string_view origReq) {
    ParseBuf_.reserve(origReq.size() + 16);
    ParseBuf_.assign(origReq.begin(), origReq.end());
    ParseBuf_.insert(ParseBuf_.end(), 15, ' ');
    ParseBuf_.push_back('\0');
    char* req = ParseBuf_.data();

    while (*req == ' ' || *req == '\t')
        req++;
    if (*req != '/')
        return false;     // we are not a proxy
    while (req[1] == '/') // remove redundant slashes
        req++;

    char* pathBegin = req;
    char* queryBegin = nullptr;

#ifdef _sse4_2_
    const __m128i simdSpace = _mm_set1_epi8(' ');
    const __m128i simdTab = _mm_set1_epi8('\t');
    const __m128i simdHash = _mm_set1_epi8('#');
    const __m128i simdQuestion = _mm_set1_epi8('?');

    auto isEnd = [=](__m128i x) {
        const auto v = _mm_or_si128(
                _mm_or_si128(
                    _mm_cmpeq_epi8(x, simdSpace), _mm_cmpeq_epi8(x, simdTab)),
                _mm_cmpeq_epi8(x, simdHash));
        return !_mm_testz_si128(v, v);
    };

    // No need for the range check because we have padding of spaces at the end.
    for (;; req += 16) {
        const auto x = _mm_loadu_si128(reinterpret_cast<const __m128i *>(req));
        const auto isQuestionSimd = _mm_cmpeq_epi8(x, simdQuestion);
        const auto isQuestion = !_mm_testz_si128(isQuestionSimd, isQuestionSimd);
        if (isEnd(x)) {
            if (isQuestion) {
                // The prospective query end and a question sign are both in the
                // current block. Need to find out which comes first.
                for (;*req != ' ' && *req != '\t' && *req != '#'; ++req) {
                    if (*req == '?') {
                        queryBegin = req + 1;
                        break;
                    }
                }
            }
            break;
        }
        if (isQuestion) {
            // Find the exact query beginning
            for (queryBegin = req; *queryBegin != '?'; ++queryBegin) {
            }
            ++queryBegin;

            break;
        }
    }

    // If we bailed out because we found query string begin. Now look for the the end of the query
    if (queryBegin) {
        for (;; req += 16) {
            const auto x = _mm_loadu_si128(reinterpret_cast<const __m128i *>(req));
            if (isEnd(x)) {
                break;
            }
        }
    }
#else
    for (;*req != ' ' && *req != '\t' && *req != '#'; ++req) {
        if (*req == '?') {
            queryBegin = req + 1;
            break;
        }
    }
#endif

    while (*req != ' ' && *req != '\t' && *req != '#') {
        ++req;
    }

    char* pathEnd = queryBegin ? queryBegin - 1 : req;
    // Make sure Path_ and Query_ are actually zero-reminated.
    *pathEnd = '\0';
    *req = '\0';
    Path_ = std::string_view{pathBegin, pathEnd};
    if (queryBegin) {
        Query_ = std::string_view{queryBegin, req};
        OrigQuery_ = Query_;
    } else {
        Query_ = {};
        OrigQuery_ = {};
    }

    return true;
}

void TBaseServerRequestData::AddHeader(const std::string& name, const std::string& value) {
    HeadersIn_[name] = value;

    if (stricmp(name.data(), "Host") == 0) {
        size_t hostLen = strcspn(value.data(), ":");
        if (value[hostLen] == ':')
            Port_ = value.substr(hostLen + 1);
        Host_ = value.substr(0, hostLen);
    }
}

void TBaseServerRequestData::SetPath(std::string path) {
    Path_ = std::move(path);
}
