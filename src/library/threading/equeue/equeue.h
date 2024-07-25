#pragma once

#include <ydb-cpp-sdk/util/thread/pool.h>
#include <ydb-cpp-sdk/library/deprecated/atomic/atomic.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <memory>

//actual queue limit will be (maxQueueSize - numBusyThreads) or 0
class TElasticQueue: public IThreadPool {
public:
    explicit TElasticQueue(std::unique_ptr<IThreadPool> slaveQueue);

    bool Add(IObjectInQueue* obj) override;
    size_t Size() const noexcept override;

    void Start(size_t threadCount, size_t maxQueueSize) override;
    void Stop() noexcept override;

    size_t ObjectCount() const;
private:
    class TDecrementingWrapper;

    bool TryIncCounter();
private:
    std::unique_ptr<IThreadPool> SlaveQueue_;
    size_t MaxQueueSize_ = 0;
    TAtomic ObjectCount_ = 0;
    TAtomic GuardCount_ = 0;
};
