#pragma once

#include <ydb-cpp-sdk/util/generic/yexception.h>

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/generic/bt_exception.h
#include <ydb-cpp-sdk/util/system/backtrace.h>

#include <utility>
========
#include <src/util/system/backtrace.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/generic/bt_exception.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/generic/bt_exception.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/backtrace.h>

#include <utility>
>>>>>>> origin/main

template <class T>
class TWithBackTrace: public T {
public:
    template <typename... Args>
    inline TWithBackTrace(Args&&... args)
        : T(std::forward<Args>(args)...)
    {
        BT_.Capture();
    }

    const TBackTrace* BackTrace() const noexcept override {
        return &BT_;
    }

private:
    TBackTrace BT_;
};
