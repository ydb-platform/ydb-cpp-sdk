#pragma once

#include <string>

namespace NUtils {

void ToLower(std::string& str);
std::string ToLower(const std::string& str);

void RemoveAll(std::string& str, char ch);

std::string Quote(std::string_view s);

bool TrySplitOn(std::string_view src, std::string_view& l, std::string_view& r, size_t pos, size_t len);

bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);

bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);

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

}