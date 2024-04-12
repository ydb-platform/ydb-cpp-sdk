#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/threading/future/wait/wait_group.h
#include <ydb-cpp-sdk/library/threading/future/core/future.h>

#include <ydb-cpp-sdk/util/generic/ptr.h>
========
#include <src/library/threading/future/core/future.h>

#include <src/util/generic/ptr.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/threading/future/wait/wait_group.h

namespace NThreading {
    namespace NWaitGroup::NImpl {
        template <class WaitPolicy>
        struct TState;

        template <class WaitPolicy>
        using TStateRef = TIntrusivePtr<TState<WaitPolicy>>;
    }

    // a helper class which allows to
    // wait for a set of futures which is
    // not known beforehand. Might be useful, e.g., for graceful shutdown:
    // while (!Stop()) {
    //     wg.Add(
    //         DoAsyncWork());
    // }
    // std::move(wg).Finish()
    //              .GetValueSync();
    //
    //
    // the folowing are equivalent:
    // {
    //   return WaitAll(futures);
    // }
    // {
    //   TWaitGroup<TWaitPolicy::TAll> wg;
    //   for (auto&& f: futures) { wg.Add(f); }
    //   return std::move(wg).Finish();
    // }

    template <class WaitPolicy>
    class TWaitGroup {
    public:
        TWaitGroup();

        // thread-safe, exception-safe
        //
        // adds the future to the set of futures to wait for
        //
        // if an exception is thrown during a call to ::Discover, the call has no effect
        //
        // accepts non-void T just for optimization
        // (so that the caller does not have to use future.IgnoreResult())
        template <class T>
        TWaitGroup& Add(const TFuture<T>& future);

        // finishes building phase
        // and returns the future that combines the futures
        // in the wait group according to WaitPolicy
        [[nodiscard]] TFuture<void> Finish() &&;

    private:
        NWaitGroup::NImpl::TStateRef<WaitPolicy> State_;
    };
}

#define INCLUDE_FUTURE_INL_H
#include "wait_group-inl.h"
#undef INCLUDE_FUTURE_INL_H
