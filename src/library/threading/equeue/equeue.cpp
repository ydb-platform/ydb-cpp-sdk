#include "equeue.h"

TElasticQueue::TElasticQueue(std::unique_ptr<IThreadPool> slaveQueue)
    : SlaveQueue_(std::move(slaveQueue))
{
}

size_t TElasticQueue::ObjectCount() const {
    return (size_t)ObjectCount_.load();
}

bool TElasticQueue::TryIncCounter() {
    if ((size_t)++GuardCount_ > MaxQueueSize_) {
        GuardCount_--;
        return false;
    }

    return true;
}



class TElasticQueue::TDecrementingWrapper: TNonCopyable, public IObjectInQueue {
public:
    TDecrementingWrapper(IObjectInQueue* realObject, TElasticQueue* queue)
        : RealObject_(realObject)
        , Queue_(queue)
    {
        Queue_->ObjectCount_++;
    }

    ~TDecrementingWrapper() override {
        --Queue_->ObjectCount_;
        --Queue_->GuardCount_;
    }
private:
    void Process(void *tsr) override {
        THolder<TDecrementingWrapper> self(this);
        RealObject_->Process(tsr);
    }
private:
    IObjectInQueue* const RealObject_;
    TElasticQueue* const Queue_;
};



bool TElasticQueue::Add(IObjectInQueue* obj) {
    if (!TryIncCounter()) {
        return false;
    }

    THolder<TDecrementingWrapper> wrapper;
    try {
        wrapper.Reset(new TDecrementingWrapper(obj, this));
    } catch (...) {
        --GuardCount_;
        throw;
    }

    if (SlaveQueue_->Add(wrapper.Get())) {
        Y_UNUSED(wrapper.Release());
        return true;
    } else {
        return false;
    }
}

void TElasticQueue::Start(size_t threadCount, size_t maxQueueSize) {
    MaxQueueSize_ = maxQueueSize;
    SlaveQueue_->Start(threadCount, maxQueueSize);
}

void TElasticQueue::Stop() noexcept {
    return SlaveQueue_->Stop();
}

size_t TElasticQueue::Size() const noexcept {
    return SlaveQueue_->Size();
}
