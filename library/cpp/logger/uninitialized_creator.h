#pragma once

#include "backend_creator.h"
#include <memory>

class TLogBackendCreatorUninitialized : public ILogBackendCreator {
public:
    void InitCustom(const std::string& type, ELogPriority priority, bool threaded);
    virtual bool Init(const IInitContext& ctx) override;
    virtual void ToJson(NJson::TJsonValue& value) const override;

    static TFactory::TRegistrator<TLogBackendCreatorUninitialized> Registrar;

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const override;
    std::unique_ptr<ILogBackendCreator> Slave;
};
