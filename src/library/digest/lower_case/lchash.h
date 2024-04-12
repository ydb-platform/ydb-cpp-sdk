#pragma once

#include "lciter.h"

#include <src/util/digest/fnv.h>
#include <string_view>

template <class T>
static inline T FnvCaseLess(const char* b, size_t l, T t = 0) noexcept {
    using TIter = TLowerCaseIterator<const char>;

    return FnvHash(TIter(b), TIter(b + l), t);
}

template <class T>
static inline T FnvCaseLess(const std::string_view& s, T t = 0) noexcept {
    return FnvCaseLess(s.data(), s.size(), t);
}
