#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/topic/impl/executor.h
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/client/topic/topic.h>
#include <src/client/common_client/impl/client.h>

#include <ydb-cpp-sdk/util/thread/pool.h>
<<<<<<< HEAD
========
#include <src/client/ydb_topic/topic.h>
#include <src/client/ydb_common_client/impl/client.h>

#include <src/util/thread/pool.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_topic/impl/executor.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_topic/impl/executor.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main


namespace NYdb::NTopic {

class IAsyncExecutor : public IExecutor {
private:
    virtual void PostImpl(std::vector<std::function<void()>>&&) = 0;
    virtual void PostImpl(std::function<void()>&&) = 0;

public:
    bool IsAsync() const override {
        return true;
    }
    // Post Implementation MUST NOT run f before it returns
    void Post(TFunction&& f) final;
};

IExecutor::TPtr CreateDefaultExecutor();


class TThreadPoolExecutor : public IAsyncExecutor {
private:
    std::shared_ptr<IThreadPool> ThreadPool;

public:
    TThreadPoolExecutor(std::shared_ptr<IThreadPool> threadPool);
    TThreadPoolExecutor(size_t threadsCount);
    ~TThreadPoolExecutor() = default;

    bool IsAsync() const override {
        return !IsFakeThreadPool;
    }

    void DoStart() override {
        if (ThreadsCount) {
            ThreadPool->Start(ThreadsCount);
        }
    }

private:
    void PostImpl(std::vector<TFunction>&& fs) override;
    void PostImpl(TFunction&& f) override;

private:
    bool IsFakeThreadPool = false;
    size_t ThreadsCount = 0;
};

class TSerialExecutor : public IAsyncExecutor, public std::enable_shared_from_this<TSerialExecutor> {
private:
    IAsyncExecutor::TPtr Executor; //!< Wrapped executor that is actually doing the job
    bool Busy = false; //!< Set if some closure was scheduled for execution and did not finish yet
    std::mutex Mutex = {};
    std::queue<TFunction> ExecutionQueue = {};

public:
    TSerialExecutor(IAsyncExecutor::TPtr executor);
    ~TSerialExecutor() = default;

private:
    void PostImpl(std::vector<TFunction>&& fs) override;
    void PostImpl(TFunction&& f) override;
    void PostNext();
};

class TSyncExecutor : public IExecutor {
public:
    void Post(TFunction&& f) final {
        return f();
    }
    bool IsAsync() const final {
        return false;
    }
    void DoStart() override {
    }
};

IExecutor::TPtr CreateGenericExecutor();

}
