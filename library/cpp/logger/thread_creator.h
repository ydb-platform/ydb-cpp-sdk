#pragma once

#include "backend_creator.h"

#include <functional>

class TOwningThreadedLogBackendCreator: public ILogBackendCreator {
public:
    TOwningThreadedLogBackendCreator(std::unique_ptr<ILogBackendCreator>&& slave);
    virtual bool Init(const IInitContext& ctx) override;
    virtual void ToJson(NJson::TJsonValue& value) const override;
    void SetQueueOverflowCallback(std::function<void()> callback);
    void SetQueueLen(size_t len);

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const override;
    std::unique_ptr<ILogBackendCreator> Slave;
    std::function<void()> QueueOverflowCallback = {};
    size_t QueueLen = 0;
};
