#include <ydb-cpp-sdk/library/cgiparam/cgiparam.h>

#include <src/library/string_utils/scan/scan.h>
#include <src/library/string_utils/quote/quote.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/singleton.h>
=======
#include <src/util/generic/singleton.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/singleton.h>
>>>>>>> origin/main

#include <string>

TCgiParameters::TCgiParameters(std::initializer_list<std::pair<std::string, std::string>> il) {
    for (const auto& item : il) {
        insert(item);
    }
}

const std::string& TCgiParameters::Get(const std::string_view name, size_t numOfValue) const noexcept {
    const auto it = Find(name, numOfValue);

    return end() == it ? Default<std::string>() : it->second;
}

bool TCgiParameters::Erase(const std::string_view name, size_t pos) {
    const auto pair = equal_range(static_cast<std::string>(name));

    for (auto it = pair.first; it != pair.second; ++it, --pos) {
        if (0 == pos) {
            erase(it);
            return true;
        }
    }

    return false;
}

bool TCgiParameters::Erase(const std::string_view name, const std::string_view val) {
    const auto pair = equal_range(static_cast<std::string>(name));

    bool found = false;
    for (auto it = pair.first; it != pair.second;) {
        if (val == it->second) {
            it = erase(it);
            found = true;
        } else {
            ++it;
        }
    }

    return found;
}

bool TCgiParameters::ErasePattern(const std::string_view name, const std::string_view pat) {
    const auto pair = equal_range(static_cast<std::string>(name));

    bool found = false;
    for (auto it = pair.first; it != pair.second;) {
        bool startsWith = std::string_view(it->second).starts_with(pat);
        if (startsWith) {
            it = erase(it);
            found = true;
        } else {
            ++it;
        }
    }

    return found;
}

size_t TCgiParameters::EraseAll(const std::string_view name) {
    size_t num = 0;

    const auto pair = equal_range(static_cast<std::string>(name));

    for (auto it = pair.first; it != pair.second; erase(it++), ++num)
        ;

    return num;
}

void TCgiParameters::JoinUnescaped(const std::string_view key, char sep, std::string_view val) {
    const auto pair = equal_range(static_cast<std::string>(key));
    auto it = pair.first;

    if (it == pair.second) { // not found
        if (val.data() != nullptr) {
            emplace_hint(it, std::string(key), std::string(val));
        }
    } else {
        std::string& dst = it->second;

        for (++it; it != pair.second; erase(it++)) {
            dst += sep;
            dst += std::string_view(it->second.data(), it->second.size());
        }

        if (val.data() != nullptr) {
            dst += sep;
            dst += val;
        }
    }
}

static inline std::string DoUnescape(const std::string_view s) {
    std::string res;

    res.resize(CgiUnescapeBufLen(s.size()));
    res.resize(CgiUnescape(res.data(), s).size());

    return res;
}

void TCgiParameters::InsertEscaped(const std::string_view name, const std::string_view value) {
    InsertUnescaped(DoUnescape(name), DoUnescape(value));
}

template <bool addAll, class F>
static inline void DoScan(const std::string_view s, F& f) {
    ScanKeyValue<addAll, '&', '='>(s, f);
}

struct TAddEscaped {
    TCgiParameters* C;

    inline void operator()(const std::string_view key, const std::string_view val) {
        C->InsertEscaped(key, val);
    }
};

void TCgiParameters::Scan(const std::string_view query, bool form) {
    Flush();
    form ? ScanAdd(query) : ScanAddAll(query);
}

void TCgiParameters::ScanAdd(const std::string_view query) {
    TAddEscaped f = {this};

    DoScan<false>(query, f);
}

void TCgiParameters::ScanAddUnescaped(const std::string_view query) {
    auto f = [this](const std::string_view key, const std::string_view val) {
        this->InsertUnescaped(key, val);
    };

    DoScan<false>(query, f);
}

void TCgiParameters::ScanAddAllUnescaped(const std::string_view query) {
    auto f = [this](const std::string_view key, const std::string_view val) {
        this->InsertUnescaped(key, val);
    };

    DoScan<true>(query, f);
}

void TCgiParameters::ScanAddAll(const std::string_view query) {
    TAddEscaped f = {this};

    DoScan<true>(query, f);
}

std::string TCgiParameters::Print() const {
    std::string res;

    res.reserve(PrintSize());
    const char* end = Print(res.data());
    res.resize(end - res.data());

    return res;
}

char* TCgiParameters::Print(char* res) const {
    if (empty()) {
        return res;
    }

    for (auto i = begin();;) {
        res = CGIEscape(res, i->first);
        *res++ = '=';
        res = CGIEscape(res, i->second);

        if (++i == end()) {
            break;
        }

        *res++ = '&';
    }

    return res;
}

size_t TCgiParameters::PrintSize() const noexcept {
    size_t res = size(); // for '&'

    for (const auto& i : *this) {
        res += CgiEscapeBufLen(i.first.size() + i.second.size()); // extra zero will be used for '='
    }

    return res;
}

std::string TCgiParameters::QuotedPrint(const char* safe) const {
    if (empty()) {
        return std::string();
    }

    std::string res;
    res.resize(PrintSize());

    char* ptr = res.data();
    for (auto i = begin();;) {
        ptr = Quote(ptr, i->first, safe);
        *ptr++ = '=';
        ptr = Quote(ptr, i->second, safe);

        if (++i == end()) {
            break;
        }

        *ptr++ = '&';
    }

    res.resize(ptr - res.data());
    return res;
}

TCgiParameters::const_iterator TCgiParameters::Find(const std::string_view name, size_t pos) const noexcept {
    const auto pair = equal_range(static_cast<std::string>(name));

    for (auto it = pair.first; it != pair.second; ++it, --pos) {
        if (0 == pos) {
            return it;
        }
    }

    return end();
}

bool TCgiParameters::Has(const std::string_view name, const std::string_view value) const noexcept {
    const auto pair = equal_range(static_cast<std::string>(name));

    for (auto it = pair.first; it != pair.second; ++it) {
        if (value == it->second) {
            return true;
        }
    }

    return false;
}

TQuickCgiParam::TQuickCgiParam(const std::string_view cgiParamStr) {
    UnescapeBuf.reserve(CgiUnescapeBufLen(cgiParamStr.size()));
    char* buf = UnescapeBuf.data();

    auto f = [this, &buf](const std::string_view key, const std::string_view val) {
        std::string_view name = CgiUnescapeBuf(buf, key);
        buf += name.size() + 1;
        std::string_view value = CgiUnescapeBuf(buf, val);
        buf += value.size() + 1;
        Y_ASSERT(buf <= UnescapeBuf.data() + UnescapeBuf.capacity() + 1 /*trailing zero*/);
        emplace(name, value);
    };

    DoScan<false>(cgiParamStr, f);

    if (buf != UnescapeBuf.data()) {
        UnescapeBuf.resize(buf - UnescapeBuf.data() - 1 /*trailing zero*/);
    }
}

const std::string_view& TQuickCgiParam::Get(const std::string_view name, size_t pos) const noexcept {
    const auto pair = equal_range(static_cast<std::string>(name));

    for (auto it = pair.first; it != pair.second; ++it, --pos) {
        if (0 == pos) {
            return it->second;
        }
    }

    return Default<std::string_view>();
}

bool TQuickCgiParam::Has(const std::string_view name, const std::string_view value) const noexcept {
    const auto pair = equal_range(static_cast<std::string>(name));

    for (auto it = pair.first; it != pair.second; ++it) {
        if (value == it->second) {
            return true;
        }
    }

    return false;
}
