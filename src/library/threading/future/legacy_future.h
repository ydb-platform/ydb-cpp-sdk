#pragma once

#include <ydb-cpp-sdk/library/threading/future/future.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/thread/factory.h>
=======
#include <src/util/thread/factory.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/thread/factory.h>
>>>>>>> origin/main

#include <functional>

namespace NThreading {
    template <typename TR, bool IgnoreException>
    class TLegacyFuture: public IThreadFactory::IThreadAble, TNonCopyable {
    public:
        typedef TR(TFunctionSignature)();
        using TFunctionObjectType = std::function<TFunctionSignature>;
        using TResult = typename TFunctionObjectType::result_type;

    private:
        TFunctionObjectType Func_;
        TPromise<TResult> Result_;
        THolder<IThreadFactory::IThread> Thread_;

    public:
        inline TLegacyFuture(const TFunctionObjectType func, IThreadFactory* pool = SystemThreadFactory())
            : Func_(func)
            , Result_(NewPromise<TResult>())
            , Thread_(pool->Run(this))
        {
        }

        inline ~TLegacyFuture() override {
            this->Join();
        }

        inline TResult Get() {
            this->Join();
            return Result_.GetValue();
        }

    private:
        inline void Join() {
            if (Thread_) {
                Thread_->Join();
                Thread_.Destroy();
            }
        }

        template <typename Result, bool IgnoreException_>
        struct TExecutor {
            static void SetPromise(TPromise<Result>& promise, const TFunctionObjectType& func) {
                if (IgnoreException_) {
                    try {
                        promise.SetValue(func());
                    } catch (...) {
                    }
                } else {
                    promise.SetValue(func());
                }
            }
        };

        template <bool IgnoreException_>
        struct TExecutor<void, IgnoreException_> {
            static void SetPromise(TPromise<void>& promise, const TFunctionObjectType& func) {
                if (IgnoreException_) {
                    try {
                        func();
                        promise.SetValue();
                    } catch (...) {
                    }
                } else {
                    func();
                    promise.SetValue();
                }
            }
        };

        void DoExecute() override {
            TExecutor<TResult, IgnoreException>::SetPromise(Result_, Func_);
        }
    };

}
