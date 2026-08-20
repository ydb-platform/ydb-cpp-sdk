#define INCLUDE_YDB_INTERNAL_H
#include "executor_impl.h"
#undef INCLUDE_YDB_INTERNAL_H

#include <library/cpp/threading/local_executor/tbb_local_executor.h>

#include <condition_variable>
#include <exception>
#include <limits>

namespace NYdb::inline V3 {

TThreadPoolExecutor::TThreadPoolExecutor(std::shared_ptr<IThreadPool> threadPool)
    : ThreadPool_(threadPool)
    , IsStarted_(true)
    , DontStop_(true)
{
}

TThreadPoolExecutor::TThreadPoolExecutor(std::shared_ptr<IThreadPool> threadPool, std::size_t threadCount, std::size_t maxQueueSize)
    : ThreadPool_(threadPool)
    , IsStarted_(false)
    , DontStop_(false)
    , ThreadCount_(threadCount)
    , MaxQueueSize_(maxQueueSize)
{
}

void TThreadPoolExecutor::DoStart() {
    if (IsStarted_) {
        return;
    }
    ThreadPool_->Start(ThreadCount_, MaxQueueSize_);
}

void TThreadPoolExecutor::Stop() {
    if (DontStop_) {
        return;
    }
    ThreadPool_->Stop();
}


void TThreadPoolExecutor::Post(std::function<void()>&& f) {
    ThreadPool_->SafeAddFunc(std::move(f));
}

bool TThreadPoolExecutor::IsAsync() const {
    return true;
}

class TTbbExecutor::TImpl {
private:
    class TTask final : public NPar::ILocallyExecutable {
    public:
        TTask(TImpl* owner, TFunction function)
            : Owner_(owner)
            , Function_(std::move(function))
        {
        }

        void LocalExec(int) override {
            Owner_->OnTaskStarted();
            try {
                Function_();
            } catch (...) {
                // The old pool was configured with SetCatching(false): an
                // exception escaping an SDK callback is a fatal contract error.
                std::terminate();
            }
        }

    private:
        TImpl* Owner_;
        TFunction Function_;
    };

public:
    TImpl(std::size_t threadCount, std::size_t maxQueueSize)
        : Executor_(ToTbbConcurrency(threadCount))
        // The SDK's default (threadCount == 0) used TAdaptiveThreadPool,
        // which deliberately ignored the queue limit.  Keep that behavior so
        // gRPC completion threads cannot block while posting responses.
        , MaxQueueSize_(threadCount == 0 ? 0 : maxQueueSize)
    {
    }

    void Start() {
        Executor_.Initialize();
        std::lock_guard guard(Mutex_);
        Running_ = true;
    }

    void Stop() {
        {
            std::lock_guard guard(Mutex_);
            if (!Running_) {
                return;
            }
            Running_ = false;
        }
        QueueSpace_.notify_all();
        Executor_.Wait();
    }

    void Post(TFunction&& function) {
        std::unique_lock guard(Mutex_);
        QueueSpace_.wait(guard, [this] {
            return !Running_ || MaxQueueSize_ == 0 || QueuedTasks_ < MaxQueueSize_;
        });
        Y_ENSURE_EX(Running_, TThreadPoolException() << "TBB executor is not running");

        if (MaxQueueSize_ != 0) {
            ++QueuedTasks_;
        }
        try {
            Executor_.Exec(MakeIntrusive<TTask>(this, std::move(function)), 0, NPar::ILocalExecutor::HIGH_PRIORITY);
        } catch (...) {
            if (MaxQueueSize_ != 0) {
                --QueuedTasks_;
                QueueSpace_.notify_one();
            }
            throw;
        }
    }

private:
    static int ToTbbConcurrency(std::size_t threadCount) {
        if (threadCount == 0) {
            return oneapi::tbb::task_arena::automatic;
        }
        // TBB reserves one arena slot for the external thread that submits
        // SDK callbacks. The remaining slots are actual async workers.
        Y_ENSURE_EX(threadCount < static_cast<std::size_t>(std::numeric_limits<int>::max()),
            TThreadPoolException() << "thread count does not fit into int");
        return static_cast<int>(threadCount + 1);
    }

    void OnTaskStarted() {
        if (MaxQueueSize_ == 0) {
            return;
        }
        {
            std::lock_guard guard(Mutex_);
            --QueuedTasks_;
        }
        QueueSpace_.notify_one();
    }

private:
    NPar::TTbbLocalExecutor<> Executor_;
    const std::size_t MaxQueueSize_;
    std::mutex Mutex_;
    std::condition_variable QueueSpace_;
    std::size_t QueuedTasks_ = 0;
    bool Running_ = false;
};

TTbbExecutor::TTbbExecutor(std::size_t threadCount, std::size_t maxQueueSize)
    : Impl_(std::make_unique<TImpl>(threadCount, maxQueueSize))
{
}

TTbbExecutor::~TTbbExecutor() {
    Impl_->Stop();
}

void TTbbExecutor::DoStart() {
    Impl_->Start();
}

void TTbbExecutor::Stop() {
    Impl_->Stop();
}

void TTbbExecutor::Post(TFunction&& f) {
    Impl_->Post(std::move(f));
}

bool TTbbExecutor::IsAsync() const {
    return true;
}

}
