#include "rotating_file_creator.h"
#include "rotating_file.h"

std::unique_ptr<TLogBackend> TRotatingFileLogBackendCreator::DoCreateLogBackend() const {
    return std::make_unique<TRotatingFileLogBackend>(Path, MaxSizeBytes, RotatedFilesCount);
}


TRotatingFileLogBackendCreator::TRotatingFileLogBackendCreator()
    : TFileLogBackendCreator("", "rotating")
{}

bool TRotatingFileLogBackendCreator::Init(const IInitContext& ctx) {
    if (!TFileLogBackendCreator::Init(ctx)) {
        return false;
    }
    ctx.GetValue("MaxSizeBytes", MaxSizeBytes);
    ctx.GetValue("RotatedFilesCount", RotatedFilesCount);
    return true;
}

ILogBackendCreator::TFactory::TRegistrator<TRotatingFileLogBackendCreator> TRotatingFileLogBackendCreator::Registrar("rotating");

void TRotatingFileLogBackendCreator::DoToJson(NJson::TJsonValue& value) const {
    TFileLogBackendCreator::DoToJson(value);
    value["MaxSizeBytes"] = MaxSizeBytes;
    value["RotatedFilesCount"] = RotatedFilesCount;
}
