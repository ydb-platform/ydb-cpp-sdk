#include "null_creator.h"
#include "null.h"

std::unique_ptr<TLogBackend> TNullLogBackendCreator::DoCreateLogBackend() const {
    return std::make_unique<TNullLogBackend>();
}

ILogBackendCreator::TFactory::TRegistrator<TNullLogBackendCreator> TNullLogBackendCreator::RegistrarDevNull("/dev/null");
ILogBackendCreator::TFactory::TRegistrator<TNullLogBackendCreator> TNullLogBackendCreator::RegistrarNull("null");


void TNullLogBackendCreator::DoToJson(NJson::TJsonValue& /*value*/) const {
}

TNullLogBackendCreator::TNullLogBackendCreator()
    : TLogBackendCreatorBase("null")
{}
