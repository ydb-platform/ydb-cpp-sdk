#pragma once

#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <deque>

#include <stack>

template <class T, class S>
class TStack: public std::stack<T, S> {
    using TBase = std::stack<T, S>;

public:
    using TBase::TBase;

    inline explicit operator bool() const noexcept {
        return !this->empty();
    }
};
