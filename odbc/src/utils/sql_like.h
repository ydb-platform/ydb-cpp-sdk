#pragma once

#include <string_view>

namespace NYdb::NOdbc {

//  SQL LIKE — '%' is any substring, '_' is any single character.
inline bool SqlLikeMatch(std::string_view text, std::string_view pattern) {
    size_t textPos = 0;
    size_t patPos = 0;
    size_t retryPat = std::string_view::npos;
    size_t retryText = 0;

    const size_t textLen = text.size();
    const size_t patLen = pattern.size();

    while (textPos < textLen) {
        if (patPos < patLen && pattern[patPos] == '%') {
            retryPat = ++patPos;
            retryText = textPos;
            continue;
        }

        if (patPos < patLen) {
            size_t nextPat = patPos + 1;
            char expected = pattern[patPos];
            bool anyCharacter = expected == '_';
            if (expected == '\\' && nextPat < patLen) {
                expected = pattern[nextPat++];
                anyCharacter = false;
            }
            if (anyCharacter || expected == text[textPos]) {
                ++textPos;
                patPos = nextPat;
                continue;
            }
        }

        if (retryPat != std::string_view::npos) {
            patPos = retryPat;
            textPos = ++retryText;
            continue;
        }

        return false;
    }

    while (patPos < patLen && pattern[patPos] == '%') {
        ++patPos;
    }
    return patPos == patLen;
}

} // namespace NYdb::NOdbc
