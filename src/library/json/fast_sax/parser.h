#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/json/fast_sax/parser.h
#include <ydb-cpp-sdk/library/json/common/defs.h>
========
#include <src/library/json/common/defs.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/json/fast_sax/parser.h

namespace NJson {
    bool ReadJsonFast(std::string_view in, TJsonCallbacks* callbacks);

    inline bool ValidateJsonFast(std::string_view in, bool throwOnError = false) {
        Y_ASSERT(false); // this method is broken, see details in IGNIETFERRO-1243. Use NJson::ValidateJson instead, or fix this one before using
        TJsonCallbacks c(throwOnError);
        return ReadJsonFast(in, &c);
    }
}
