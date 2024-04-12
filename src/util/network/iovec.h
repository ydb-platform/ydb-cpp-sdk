#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/system/types.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
=======
#include <src/util/stream/output.h>
#include <src/util/system/types.h>
#include <src/util/system/yassert.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TContIOVector {
    using TPart = IOutputStream::TPart;

public:
    inline TContIOVector(TPart* parts, size_t count)
        : Parts_(parts)
        , Count_(count)
    {
    }

    inline void Proceed(size_t len) noexcept {
        while (Count_) {
            if (len < Parts_->len) {
                Parts_->len -= len;
                Parts_->buf = (const char*)Parts_->buf + len;

                return;
            } else {
                len -= Parts_->len;
                --Count_;
                ++Parts_;
            }
        }

        if (len) {
            Y_ASSERT(0 && "non zero length left");
        }
    }

    inline const TPart* Parts() const noexcept {
        return Parts_;
    }

    inline size_t Count() const noexcept {
        return Count_;
    }

    static inline size_t Bytes(const TPart* parts, size_t count) noexcept {
        size_t ret = 0;

        for (size_t i = 0; i < count; ++i) {
            ret += parts[i].len;
        }

        return ret;
    }

    inline size_t Bytes() const noexcept {
        return Bytes(Parts_, Count_);
    }

    inline bool Complete() const noexcept {
        return !Count();
    }

private:
    TPart* Parts_;
    size_t Count_;
};
