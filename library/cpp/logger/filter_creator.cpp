#include "filter_creator.h"
#include "filter.h"

TFilteredBackendCreator::TFilteredBackendCreator(std::unique_ptr<ILogBackendCreator> slave, ELogPriority priority)
    : Slave(std::move(slave))
    , Priority(priority)
{}

std::unique_ptr<TLogBackend> TFilteredBackendCreator::DoCreateLogBackend() const {
    return MakeHolder<TFilteredLogBackend>(Slave->CreateLogBackend(), Priority);
}

bool TFilteredBackendCreator::Init(const IInitContext& ctx) {
    return Slave->Init(ctx);
}

void TFilteredBackendCreator::ToJson(NJson::TJsonValue& value) const {
    value["LogLevel"] = ToString(Priority).c_str();
    Slave->ToJson(value);
}
