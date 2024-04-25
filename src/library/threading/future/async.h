#pragma once

#include <ydb-cpp-sdk/library/threading/future/future.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/function.h>
#include <ydb-cpp-sdk/util/thread/pool.h>
=======
#include <src/util/generic/function.h>
#include <src/util/thread/pool.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/function.h>
#include <ydb-cpp-sdk/util/thread/pool.h>
>>>>>>> origin/main

namespace NThreading {
    /**
 * @brief Asynchronously executes @arg func in @arg queue returning a future for the result.
 *
 * @arg func should be a callable object with signature T().
 * @arg queue where @arg will be executed
 * @returns For @arg func with signature T() the function returns TFuture<T> unless T is TFuture<U>.
 *          In this case the function returns TFuture<U>.
 *
 * If you want to use another queue for execution just write an overload, @see ExtensionExample
 * unittest.
 */
    template <typename Func>
    TFuture<TFutureType<TFunctionResult<Func>>> Async(Func&& func, IThreadPool& queue) {
        auto promise = NewPromise<TFutureType<TFunctionResult<Func>>>();
        auto lambda = [promise, func = std::forward<Func>(func)]() mutable {
            NImpl::SetValue(promise, func);
        };
        queue.SafeAddFunc(std::move(lambda));

        return promise.GetFuture();
    }

}
