#pragma once

#include <string_view>

namespace NYdb::NOdbc {

//  SQL LIKE — '%' is any substring, '_' is any single character.
inline bool SqlLikeMatch(std::string_view text, std::string_view pattern) {
    size_t textPos = 0;
    size_t patPos = 0;
    size_t lastPercentPat = std::string_view::npos;
    size_t textStartAfterPercent = 0;

    const size_t textLen = text.size();
    const size_t patLen = pattern.size();

    while (textPos < textLen) {
        const bool morePat = patPos < patLen;
        const char patCh = morePat ? pattern[patPos] : '\0';

        if (morePat && patCh != '%' && (patCh == '_' || patCh == text[textPos])) {
            ++textPos;
            ++patPos;
            continue;
        }

        if (morePat && patCh == '%') {
            lastPercentPat = patPos++;
            textStartAfterPercent = textPos;
            continue;
        }

        if (lastPercentPat != std::string_view::npos) {
            patPos = lastPercentPat + 1;
            ++textStartAfterPercent;
            textPos = textStartAfterPercent;
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
