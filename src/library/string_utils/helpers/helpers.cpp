#include <ydb-cpp-sdk/library/string_utils/helpers/helpers.h>

#include <algorithm>

namespace NUtils {
    namespace {
    void DoSplit(std::string_view src, std::string_view& l, std::string_view& r, size_t pos, size_t len) {
        const auto right = src.substr(pos + len); // in case if (&l == &src)
        l = src.substr(0, pos);
        r = right;
    }
}

void ToLower(std::string& str) {
    for (char& c: str) {
        c = std::tolower(c);
    }
}

std::string ToLower(const std::string& str) {
    std::string ret(str);
    ToLower(ret);
    return ret;
}

std::string ToTitle(const std::string& s) {
    std::string res = ToLower(s);
    if (!res.empty()) {
        res[0] = std::toupper(res[0]);
    }
    return res;
}

void RemoveAll(std::string& str, char ch) {
    size_t pos = str.find(ch); // 'find' to avoid cloning of string in 'TString.begin()'
    if (pos == std::string::npos)
        return;

    auto begin = str.begin();
    auto end = begin + str.size();
    auto it = std::remove(begin + pos, end, ch);
    str.erase(it, end);
}

bool TrySplitOn(std::string_view src, std::string_view& l, std::string_view& r, size_t pos, size_t len) {
    if (pos == std::string_view::npos) {
        return false;
    }
    DoSplit(src, l, r, pos, len);
    return true;
}

bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, char delim) {
    return TrySplitOn(src, l, r, src.find(delim), 1);
}

bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim) {
    return TrySplitOn(src, l, r, src.find(delim), delim.size());
}

bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, char delim) {
    return TrySplitOn(src, l, r, src.rfind(delim), 1);
}

bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim) {
    return TrySplitOn(src, l, r, src.rfind(delim), delim.size());
}

void Split(std::string_view src, std::string_view& l, std::string_view& r, char delim) {
    if (!TrySplit(src, l, r, delim)) {
        l = src;
        r = {};
    }
}

void Split(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim) {
    if (!TrySplit(src, l, r, delim)) {
        l = src;
        r = {};
    }
}

void RSplit(std::string_view src, std::string_view& l, std::string_view& r, char delim) {
    if (!TryRSplit(src, l, r, delim)) {
        r = src;
        l = {};
    }
}

void RSplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim) {
    if (!TryRSplit(src, l, r, delim)) {
        r = src;
        l = {};
    }
}

std::string_view NextTok(std::string_view& src, char delim) {
    std::string_view tok;
    Split(src, tok, src, delim);
    return tok;
}

std::string_view NextTok(std::string_view& src, std::string_view delim) {
    std::string_view tok;
    Split(src, tok, src, delim);
    return tok;
}

bool NextTok(std::string_view& src, std::string_view& tok, char delim) {
    if (!src.empty()) {
        Split(src, tok, src, delim);
        return true;
    }
    return false;
}

bool NextTok(std::string_view& src, std::string_view& tok, std::string_view delim) {
    if (!src.empty()) {
        Split(src, tok, src, delim);
        return true;
    }
    return false;
}

std::string_view RNextTok(std::string_view& src, char delim) {
    std::string_view tok;
    RSplit(src, src, tok, delim);
    return tok;
}

std::string_view RNextTok(std::string_view& src, std::string_view delim) {
    std::string_view tok;
    RSplit(src, src, tok, delim);
    return tok;
}

std::string_view After(std::string_view src, char c) {
    std::string_view l, r;
    return TrySplit(src, l, r, c) ? r : src;
}

std::string_view Before(std::string_view src, char c) {
    std::string_view l, r;
    return TrySplit(src, l, r, c) ? l : src;
}

size_t SumLength() noexcept {
    return 0;
}

void CopyAll(char*) noexcept {
}

template <>
std::u16string FromAscii(const ::std::string_view& s) {
    std::u16string res;
    res.resize(s.size());

    auto dst = res.begin();

    for (const char* src = s.data(); dst != res.end(); ++dst, ++src) {
        *dst = static_cast<char16_t>(*src);
    }

    return res;
}

template <>
std::u32string FromAscii(const ::std::string_view& s) {
    std::u32string res;
    res.resize(s.size());

    auto dst = res.begin();

    for (const char* src = s.data(); dst != res.end(); ++dst, ++src) {
        *dst = static_cast<char32_t>(*src);
    }

    return res;
}

}
