#pragma once

#include <util/generic/strbuf.h>

template <bool addAll, char sep, char sepKeyVal, class F>
static inline void ScanKeyValue(std::string_view s, F&& f) {
    std::string_view key, val;
    TStringBuf().NextTok(' ');

    while (!s.empty()) {
        auto splitPos = s.find(sep);
        val = s.substr(0, splitPos);
        if (splitPos == std::string_view::npos) {
            s = {};
        } else {
            s = s.substr(splitPos + 1);
        }

        if (val.empty()) {
            continue; // && case
        }

        splitPos = s.find(sepKeyVal);
        key = s.substr(0, splitPos);
        if (splitPos == std::string_view::npos) {
            val = {};
        } else {
            val = val.substr(splitPos + 1);
        }

        if (addAll || val.data() != nullptr) {
            f(key, val); // includes empty keys
        }
    }
}
