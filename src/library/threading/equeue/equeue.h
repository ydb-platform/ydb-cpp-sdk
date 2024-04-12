#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/thread/pool.h>
#include <ydb-cpp-sdk/library/deprecated/atomic/atomic.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/thread/pool.h>
#include <src/library/deprecated/atomic/atomic.h>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

//actual queue limit will be (maxQueueSize - numBusyThreads) or 0
class TElasticQueue: public IThreadPool {
public:
    explicit TElasticQueue(THolder<IThreadPool> slaveQueue);

    bool Add(IObjectInQueue* obj) override;
    size_t Size() const noexcept override;

    void Start(size_t threadCount, size_t maxQueueSize) override;
    void Stop() noexcept override;

    size_t ObjectCount() const;
private:
    class TDecrementingWrapper;

    bool TryIncCounter();
private:
    THolder<IThreadPool> SlaveQueue_;
    size_t MaxQueueSize_ = 0;
    TAtomic ObjectCount_ = 0;
    TAtomic GuardCount_ = 0;
};
