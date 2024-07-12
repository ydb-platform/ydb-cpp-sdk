#include "file_creator.h"
#include "file.h"

#include <memory>

TFileLogBackendCreator::TFileLogBackendCreator(const std::string& path /*= std::string()*/, const std::string& type /*= "file"*/)
    : TLogBackendCreatorBase(type)
    , Path(path)
{}

std::unique_ptr<TLogBackend> TFileLogBackendCreator::DoCreateLogBackend() const {
    return std::make_unique<TFileLogBackend>(Path);
}

bool TFileLogBackendCreator::Init(const IInitContext& ctx) {
    ctx.GetValue("Path", Path);
    return !Path.empty();
}

ILogBackendCreator::TFactory::TRegistrator<TFileLogBackendCreator> TFileLogBackendCreator::Registrar("file");

void TFileLogBackendCreator::DoToJson(NJson::TJsonValue& value) const {
    value["Path"] = Path;
}
