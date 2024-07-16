#pragma once

#include "backend_creator.h"


class TCompositeBackendCreator : public TLogBackendCreatorBase {
public:
    TCompositeBackendCreator();
    virtual bool Init(const IInitContext& ctx) override;
    static TFactory::TRegistrator<TCompositeBackendCreator> Registrar;

protected:
    virtual void DoToJson(NJson::TJsonValue& value) const override;

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const override;
    std::vector<std::unique_ptr<ILogBackendCreator>> Children;
};
