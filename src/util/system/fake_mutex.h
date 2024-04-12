#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
=======
#include <src/util/generic/noncopyable.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TFakeMutex: public TNonCopyable {
public:
    inline void Acquire() noexcept {
    }

    inline bool TryAcquire() noexcept {
        return true;
    }

    inline void Release() noexcept {
    }

    inline void lock() noexcept {
        Acquire();
    }

    inline bool try_lock() noexcept {
        return TryAcquire();
    }

    inline void unlock() noexcept {
        Release();
    }

    ~TFakeMutex() = default;
};