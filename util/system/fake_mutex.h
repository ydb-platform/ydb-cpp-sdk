#include <util/generic/noncopyable.h>

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