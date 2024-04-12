#pragma once

#include <ydb-cpp-sdk/util/generic/yexception.h>

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/generic/bt_exception.h
#include <ydb-cpp-sdk/util/system/backtrace.h>

#include <utility>
========
#include <src/util/system/backtrace.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/generic/bt_exception.h

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
