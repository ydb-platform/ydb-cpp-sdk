#pragma once

#include "backend_creator.h"

class TFileLogBackendCreator : public TLogBackendCreatorBase {
public:
    TFileLogBackendCreator(const std::string& path = std::string(), const std::string& type = "file");
    virtual bool Init(const IInitContext& ctx) override;
    static TFactory::TRegistrator<TFileLogBackendCreator> Registrar;

protected:
    virtual void DoToJson(NJson::TJsonValue& value) const override;
    std::string Path;

private:
    virtual THolder<TLogBackend> DoCreateLogBackend() const override;
};
