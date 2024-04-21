#pragma once

#include "backend_creator.h"

class TCerrLogBackendCreator : public TLogBackendCreatorBase {
public:
    TCerrLogBackendCreator();
    static TFactory::TRegistrator<TCerrLogBackendCreator> RegistrarCerr;
    static TFactory::TRegistrator<TCerrLogBackendCreator> RegistrarConsole;

protected:
    virtual void DoToJson(NJson::TJsonValue& value) const override;

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const override;
};

class TCoutLogBackendCreator : public TLogBackendCreatorBase {
public:
    TCoutLogBackendCreator();
    static TFactory::TRegistrator<TCoutLogBackendCreator> Registrar;

protected:
    virtual void DoToJson(NJson::TJsonValue& value) const override;

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const override;
};
