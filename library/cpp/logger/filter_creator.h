#pragma once

#include "backend_creator.h"
#include "priority.h"

class TFilteredBackendCreator : public ILogBackendCreator {
public:
    TFilteredBackendCreator(std::unique_ptr<ILogBackendCreator> slave, ELogPriority priority);
    virtual bool Init(const IInitContext& ctx) override;
    virtual void ToJson(NJson::TJsonValue& value) const override;

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const override;
    std::unique_ptr<ILogBackendCreator> Slave;
    ELogPriority Priority;
};
