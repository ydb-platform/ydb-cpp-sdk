#pragma once

#include <span>

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/threading/future/wait/wait.h
#include <ydb-cpp-sdk/library/threading/future/core/future.h>

#include "wait_group.h"
========
#include <src/library/threading/future/core/future.h>
#include <src/library/threading/future/wait/wait_group.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/threading/future/wait/wait.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/threading/future/wait/wait.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/threading/future/core/future.h>

#include "wait_group.h"
>>>>>>> origin/main

namespace NThreading {
    namespace NImpl {
        template <class TContainer>
        using EnableGenericWait = std::enable_if_t<
            !std::is_convertible_v<TContainer, std::span<const TFuture<void>>>,
            TFuture<void>>;
    }
    // waits for all futures
    [[nodiscard]] TFuture<void> WaitAll(const TFuture<void>& f1);
    [[nodiscard]] TFuture<void> WaitAll(const TFuture<void>& f1, const TFuture<void>& f2);
    [[nodiscard]] TFuture<void> WaitAll(std::span<const TFuture<void>> futures);
    template <typename TContainer>
    [[nodiscard]] NImpl::EnableGenericWait<TContainer> WaitAll(const TContainer& futures);

    // waits for the first exception or for all futures
    [[nodiscard]] TFuture<void> WaitExceptionOrAll(const TFuture<void>& f1);
    [[nodiscard]] TFuture<void> WaitExceptionOrAll(const TFuture<void>& f1, const TFuture<void>& f2);
    [[nodiscard]] TFuture<void> WaitExceptionOrAll(std::span<const TFuture<void>> futures);
    template <typename TContainer>
    [[nodiscard]] NImpl::EnableGenericWait<TContainer> WaitExceptionOrAll(const TContainer& futures);

    // waits for any future
    [[nodiscard]] TFuture<void> WaitAny(const TFuture<void>& f1);
    [[nodiscard]] TFuture<void> WaitAny(const TFuture<void>& f1, const TFuture<void>& f2);
    [[nodiscard]] TFuture<void> WaitAny(std::span<const TFuture<void>> futures);
    template <typename TContainer>
    [[nodiscard]] NImpl::EnableGenericWait<TContainer> WaitAny(const TContainer& futures);
}

#define INCLUDE_FUTURE_INL_H
#include "wait-inl.h"
#undef INCLUDE_FUTURE_INL_H
