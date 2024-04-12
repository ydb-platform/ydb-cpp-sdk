#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/deprecated/atomic/atomic.h>
=======
#include <src/library/deprecated/atomic/atomic.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NAtomic {
    class TBool {
    public:
        TBool() noexcept = default;

        TBool(bool val) noexcept
            : Val_(val)
        {
        }

        TBool(const TBool& src) noexcept {
            AtomicSet(Val_, AtomicGet(src.Val_));
        }

        operator bool() const noexcept {
            return AtomicGet(Val_);
        }

        const TBool& operator=(bool val) noexcept {
            AtomicSet(Val_, val);
            return *this;
        }

        const TBool& operator=(const TBool& src) noexcept {
            AtomicSet(Val_, AtomicGet(src.Val_));
            return *this;
        }

    private:
        TAtomic Val_ = 0;
    };
}
