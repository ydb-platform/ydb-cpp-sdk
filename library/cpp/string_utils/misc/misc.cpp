#include "misc.h"

#include <library/cpp/string_builder/string_builder.h>
#include <library/cpp/string_utils/escape/escape.h>

namespace NUtils {

void ToLower(std::string& str) {
    for (char& c: str) {
        c = std::tolower(c);
    }
}

std::string TStringQuote(std::string_view s) {
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

}