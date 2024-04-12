#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/threading/future/future.h>

#include <ydb-cpp-sdk/util/system/spinlock.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/library/threading/future/future.h>

#include <src/util/system/spinlock.h>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <list>
#include <functional>

namespace NThreading {

class TAsyncSemaphore: public TThrRefBase {
public:
    using TPtr = TIntrusivePtr<TAsyncSemaphore>;

    class TAutoRelease {
    public:
        TAutoRelease(TAsyncSemaphore::TPtr sem)
            : Sem(std::move(sem))
        {
        }
        TAutoRelease(TAutoRelease&& other)
            : Sem(std::move(other.Sem))
        {
        }
        ~TAutoRelease();

        std::function<void (const TFuture<void>&)> DeferRelease();

    private:
        TAsyncSemaphore::TPtr Sem;
    };

    static TPtr Make(size_t count);

    TFuture<TPtr> AcquireAsync();
    void Release();
    void Cancel();

    TAutoRelease MakeAutoRelease() {
        return {this};
    }

private:
    TAsyncSemaphore(size_t count);

private:
    size_t Count_;
    bool Cancelled_ = false;
    TAdaptiveLock Lock_;
    std::list<TPromise<TPtr>> Promises_;
};

} // namespace NThreading
