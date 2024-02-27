#pragma once

#include <optional>
#include <string>

#include <util/generic/yexception.h>
#include <util/string/cast.h>

namespace NUtils {

std::string ToLower(const std::string& str);
void ToLower(std::string& str);

std::string ToTitle(const std::string& s);

void RemoveAll(std::string& str, char ch);

std::string Quote(std::string_view s);

bool TrySplitOn(std::string_view src, std::string_view& l, std::string_view& r, size_t pos, size_t len);

bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);
bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);

void Split(std::string_view src, std::string_view& l, std::string_view& r, char delim);
void Split(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);
void RSplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
void RSplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);

std::string_view NextTok(std::string_view& src, char delim);
std::string_view NextTok(std::string_view& src, std::string_view delim);
bool NextTok(std::string_view& src, std::string_view& tok, char delim);
bool NextTok(std::string_view& src, std::string_view& tok, std::string_view delim);

std::string_view RNextTok(std::string_view& src, char delim);
std::string_view RNextTok(std::string_view& src, std::string_view delim);

std::string_view After(std::string_view src, char c);
std::string_view Before(std::string_view src, char c);

template <class TContainer, class T>
bool ContainerTransform(TContainer& str, T&& f, size_t pos = 0, size_t n = TContainer::npos) {
    size_t len = str.size();
    if (pos > len) {
        pos = len;
    }
    if (n > len - pos) {
        n = len - pos;
    }

    bool changed = false;

    for (size_t i = pos; i != pos + n; ++i) {
        auto c = f(i, str[i]);
        if (c != str[i]) {
            changed = true;
            str[i] = c;
        }
    }
    return changed;
}

template <class P, class D>
void GetNext(std::string_view& s, D delim, P& param) {
    std::string_view next;
    Y_ENSURE(NUtils::NextTok(s, next, delim), "Split: number of fields less than number of Split output arguments");
    param = FromString<P>(next);
}

template <class P, class D>
void GetNext(std::string_view& s, D delim, std::optional<P>& param) {
    std::string_view next;
    if (NUtils::NextTok(s, next, delim)) {
        param = FromString<P>(next);
    } else {
        param.reset();
    }
}

}