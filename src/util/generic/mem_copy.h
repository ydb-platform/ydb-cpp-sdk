#pragma once

#include <ydb-cpp-sdk/util/generic/typetraits.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
=======
#include <src/util/system/types.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/types.h>
>>>>>>> origin/main

#include <cstring>

template <class T>
using TIfPOD = std::enable_if_t<TTypeTraits<T>::IsPod, T*>;

template <class T>
using TIfNotPOD = std::enable_if_t<!TTypeTraits<T>::IsPod, T*>;

template <class T>
static inline TIfPOD<T> MemCopy(T* to, const T* from, size_t n) noexcept {
    if (n) {
        memcpy(to, from, n * sizeof(T));
    }

    return to;
}

template <class T>
static inline TIfNotPOD<T> MemCopy(T* to, const T* from, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        to[i] = from[i];
    }

    return to;
}

template <class T>
static inline TIfPOD<T> MemMove(T* to, const T* from, size_t n) noexcept {
    if (n) {
        memmove(to, from, n * sizeof(T));
    }

    return to;
}

template <class T>
static inline TIfNotPOD<T> MemMove(T* to, const T* from, size_t n) {
    if (to <= from || to >= from + n) {
        return MemCopy(to, from, n);
    }

    //copy backwards
    while (n) {
        to[n - 1] = from[n - 1];
        --n;
    }

    return to;
}
