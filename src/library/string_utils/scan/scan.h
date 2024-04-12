#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/string_utils/helpers/helpers.h>
=======
#include <src/library/string_utils/helpers/helpers.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
