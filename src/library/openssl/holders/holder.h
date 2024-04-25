#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main

namespace NOpenSSL {

template <typename TType, auto Create, auto Destroy, class... Args>
class THolder {
public:
    inline THolder(Args... args) {
        Ptr = Create(args...);
        if (!Ptr) {
            throw std::bad_alloc();
        }
    }

    THolder(const THolder&) = delete;
    THolder& operator=(const THolder&) = delete;

    inline ~THolder() noexcept {
        Destroy(Ptr);
    }

    inline operator TType* () noexcept {
        return Ptr;
    }

private:
    TType* Ptr;
};

}
