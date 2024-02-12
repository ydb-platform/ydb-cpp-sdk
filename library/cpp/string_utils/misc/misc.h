#pragma once

#include <string>

namespace NUtils {

void ToLower(std::string& str);
void RemoveAll(std::string& str, char ch);
std::string TStringQuote(std::string_view s);

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