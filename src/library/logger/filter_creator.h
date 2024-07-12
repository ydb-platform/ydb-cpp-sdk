#pragma once

#include "backend_creator.h"
#include <ydb-cpp-sdk/library/logger/priority.h>

class TFilteredBackendCreator : public ILogBackendCreator {
public:
    TFilteredBackendCreator(THolder<ILogBackendCreator> slave, ELogPriority priority);
    virtual bool Init(const IInitContext& ctx) override;
    virtual void ToJson(NJson::TJsonValue& value) const override;

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const override;
    THolder<ILogBackendCreator> Slave;
    ELogPriority Priority;
};
