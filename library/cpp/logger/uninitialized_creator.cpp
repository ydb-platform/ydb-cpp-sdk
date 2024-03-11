#include "uninitialized_creator.h"
#include "filter_creator.h"
#include "thread_creator.h"
#include "file_creator.h"
#include "null_creator.h"
#include <util/string/cast.h>

std::unique_ptr<TLogBackend> TLogBackendCreatorUninitialized::DoCreateLogBackend() const {
    return Slave->CreateLogBackend();
}

void TLogBackendCreatorUninitialized::InitCustom(const std::string& type, ELogPriority priority, bool threaded) {
    if (type.empty()) {
        Slave = std::make_unique<TNullLogBackendCreator>();
    } else if (TFactory::Has(type)) {
        Slave = TFactory::MakeHolder(type);
    } else {
        Slave = std::make_unique<TFileLogBackendCreator>(type);
    }

    if (threaded) {
        Slave = std::make_unique<TOwningThreadedLogBackendCreator>(std::move(Slave));
    }

    if (priority != LOG_MAX_PRIORITY) {
        Slave = std::make_unique<TFilteredBackendCreator>(std::move(Slave), priority);
    }
}

bool TLogBackendCreatorUninitialized::Init(const IInitContext& ctx) {
    auto type = ctx.GetOrElse("LoggerType", std::string());
    bool threaded = ctx.GetOrElse("Threaded", false);
    ELogPriority priority = LOG_MAX_PRIORITY;
    std::string prStr;
    if (ctx.GetValue("LogLevel", prStr)) {
        if (!TryFromString(prStr, priority)) {
            priority = (ELogPriority)FromString<int>(prStr);
        }
    }
    InitCustom(type, priority, threaded);
    return Slave->Init(ctx);
}


void TLogBackendCreatorUninitialized::ToJson(NJson::TJsonValue& value) const {
    Y_ABORT_UNLESS(Slave, "Serialization off uninitialized LogBackendCreator");
    Slave->ToJson(value);
}

ILogBackendCreator::TFactory::TRegistrator<TLogBackendCreatorUninitialized> TLogBackendCreatorUninitialized::Registrar("");
