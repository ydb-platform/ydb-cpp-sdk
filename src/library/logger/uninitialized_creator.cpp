#include "uninitialized_creator.h"
#include "filter_creator.h"
#include "thread_creator.h"
#include "file_creator.h"
#include "null_creator.h"
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/cast.h>
=======
#include <src/util/string/cast.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

THolder<TLogBackend> TLogBackendCreatorUninitialized::DoCreateLogBackend() const {
    return Slave->CreateLogBackend();
}

void TLogBackendCreatorUninitialized::InitCustom(const std::string& type, ELogPriority priority, bool threaded) {
    if (type.empty()) {
        Slave = MakeHolder<TNullLogBackendCreator>();
    } else if (TFactory::Has(type)) {
        Slave = TFactory::MakeHolder(type);
    } else {
        Slave = MakeHolder<TFileLogBackendCreator>(type);
    }

    if (threaded) {
        Slave = MakeHolder<TOwningThreadedLogBackendCreator>(std::move(Slave));
    }

    if (priority != LOG_MAX_PRIORITY) {
        Slave = MakeHolder<TFilteredBackendCreator>(std::move(Slave), priority);
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
