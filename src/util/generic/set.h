#pragma once

#include <ydb-cpp-sdk/util/generic/fwd.h>

#include <ydb-cpp-sdk/util/str_stl.h>
#include <ydb-cpp-sdk/util/memory/alloc.h>

#include <initializer_list>
#include <memory>
#include <set>

template <class K, class L, class A>
class TSet: public std::set<K, L, TReboundAllocator<A, K>> {
public:
    using TBase = std::set<K, L, TReboundAllocator<A, K>>;
    using TBase::TBase;

    inline explicit operator bool() const noexcept {
        return !this->empty();
    }
};

template <class K, class L, class A>
class TMultiSet: public std::multiset<K, L, TReboundAllocator<A, K>> {
public:
    using TBase = std::multiset<K, L, TReboundAllocator<A, K>>;
    using TBase::TBase;

    inline explicit operator bool() const noexcept {
        return !this->empty();
    }
};
