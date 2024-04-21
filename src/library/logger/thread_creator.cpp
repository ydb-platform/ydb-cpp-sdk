#include "thread_creator.h"
#include <ydb-cpp-sdk/library/logger/thread.h>

TOwningThreadedLogBackendCreator::TOwningThreadedLogBackendCreator(std::unique_ptr<ILogBackendCreator>&& slave)
    : Slave(std::move(slave))
{}

std::unique_ptr<TLogBackend> TOwningThreadedLogBackendCreator::DoCreateLogBackend() const {
    return MakeHolder<TOwningThreadedLogBackend>(Slave->CreateLogBackend().release(), QueueLen, QueueOverflowCallback);
}

bool TOwningThreadedLogBackendCreator::Init(const IInitContext& ctx) {
    ctx.GetValue("QueueLen", QueueLen);
    return Slave->Init(ctx);
}


void TOwningThreadedLogBackendCreator::ToJson(NJson::TJsonValue& value) const {
    value["QueueLen"] =  QueueLen;
    value["Threaded"] = true;
    Slave->ToJson(value);
}

void TOwningThreadedLogBackendCreator::SetQueueOverflowCallback(std::function<void()> callback) {
    QueueOverflowCallback = std::move(callback);
}

void TOwningThreadedLogBackendCreator::SetQueueLen(size_t len) {
    QueueLen = len;
}
