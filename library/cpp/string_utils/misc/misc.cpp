#include "misc.h"

#include <library/cpp/string_builder/string_builder.h>
#include <library/cpp/string_utils/escape/escape.h>

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

std::string Quote(std::string_view s) {
    return TYdbStringBuilder() << "\"" << EscapeC(s) << "\"";
}

void RemoveAll(std::string& str, char ch) {
    size_t pos = str.find(ch); // 'find' to avoid cloning of string in 'TString.begin()'
    if (pos == std::string::npos)
        return;

    auto begin = str.begin();
    auto end = begin + str.length();
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

std::string_view After(std::string_view src, char c) {
    std::string_view l, r;
    return TrySplit(src, l, r, c) ? r : src;
}

std::string_view Before(std::string_view src, char c) {
    std::string_view l, r;
    return TrySplit(src, l, r, c) ? l : src;
}

}