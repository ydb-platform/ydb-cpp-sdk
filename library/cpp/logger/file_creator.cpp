#include "file_creator.h"
#include "file.h"

TFileLogBackendCreator::TFileLogBackendCreator(const std::string& path /*= std::string()*/, const std::string& type /*= "file"*/)
    : TLogBackendCreatorBase(type)
    , Path(path)
{}

THolder<TLogBackend> TFileLogBackendCreator::DoCreateLogBackend() const {
    return MakeHolder<TFileLogBackend>(Path);
}

bool TFileLogBackendCreator::Init(const IInitContext& ctx) {
    ctx.GetValue("Path", Path);
    return !!Path;
}

ILogBackendCreator::TFactory::TRegistrator<TFileLogBackendCreator> TFileLogBackendCreator::Registrar("file");

void TFileLogBackendCreator::DoToJson(NJson::TJsonValue& value) const {
    value["Path"] = Path;
}
