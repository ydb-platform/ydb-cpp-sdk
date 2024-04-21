#pragma once

#include <ydb-cpp-sdk/library/string_utils/helpers/helpers.h>

#include <string_view>

template <bool addAll, char sep, char sepKeyVal, class F>
static inline void ScanKeyValue(std::string_view s, F&& f) {
    std::string_view key, val;

    while (!s.empty()) {
        val = NUtils::NextTok(s, sep);

        if (val.empty()) {
            continue; // && case
        }

        key = NUtils::NextTok(val, sepKeyVal);

        if (addAll || val.data() != nullptr) {
            f(key, val); // includes empty keys
        }
    }
}
