#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/system/defaults.h>
=======
#include "yassert.h"
#include "defaults.h"
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/system/defaults.h>
>>>>>>> origin/main
#include <src/util/generic/bitops.h>

template <class T>
static inline T AlignDown(T len, T align) noexcept {
    Y_ASSERT(IsPowerOf2(align)); // align should be power of 2
    return len & ~(align - 1);
}

template <class T>
static inline T AlignUp(T len, T align) noexcept {
    const T alignedResult = AlignDown(len + (align - 1), align);
    Y_ASSERT(alignedResult >= len); // check for overflow
    return alignedResult;
}

template <class T>
static inline T AlignUpSpace(T len, T align) noexcept {
    Y_ASSERT(IsPowerOf2(align));       // align should be power of 2
    return ((T)0 - len) & (align - 1); // AlignUp(len, align) - len;
}

template <class T>
static inline T* AlignUp(T* ptr, size_t align) noexcept {
    return (T*)AlignUp((uintptr_t)ptr, align);
}

template <class T>
static inline T* AlignDown(T* ptr, size_t align) noexcept {
    return (T*)AlignDown((uintptr_t)ptr, align);
}

template <class T>
static inline T AlignUp(T t) noexcept {
    return AlignUp(t, (size_t)PLATFORM_DATA_ALIGN);
}

template <class T>
static inline T AlignDown(T t) noexcept {
    return AlignDown(t, (size_t)PLATFORM_DATA_ALIGN);
}

template <class T>
static inline T Align(T t) noexcept {
    return AlignUp(t);
}
