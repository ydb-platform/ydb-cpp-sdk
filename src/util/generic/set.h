#pragma once

#include <ydb-cpp-sdk/util/generic/fwd.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/str_stl.h>
#include <ydb-cpp-sdk/util/memory/alloc.h>
=======
#include <src/util/str_stl.h>
#include <src/util/memory/alloc.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
