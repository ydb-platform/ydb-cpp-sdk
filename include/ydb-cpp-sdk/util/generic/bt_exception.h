#pragma once

#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <ydb-cpp-sdk/util/system/backtrace.h>

#include <utility>

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
