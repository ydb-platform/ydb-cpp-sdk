#pragma once

#include "numeric.h"
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/digest/multi.h
#include <ydb-cpp-sdk/util/str_stl.h>
========
#include <src/util/str_stl.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/digest/multi.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/digest/multi.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

template <typename TOne>
constexpr size_t MultiHash(const TOne& one) noexcept {
    return THash<TOne>()(one);
}

template <typename THead, typename... TTail>
constexpr size_t MultiHash(const THead& head, const TTail&... tail) noexcept {
    return CombineHashes(MultiHash(tail...), THash<THead>()(head));
}
