#include "global.h"

static void DoInitGlobalLog(std::unique_ptr<TGlobalLog> logger, std::unique_ptr<ILoggerFormatter> formatter) {
    TLoggerOperator<TGlobalLog>::Set(logger.release());
    if (!formatter) {
        formatter.reset(CreateRtyLoggerFormatter());
    }
    TLoggerFormatterOperator::Set(formatter.release());
}

void DoInitGlobalLog(const std::string& logType, const int logLevel, const bool rotation, const bool startAsDaemon, std::unique_ptr<ILoggerFormatter> formatter, bool threaded) {
    DoInitGlobalLog(
        std::make_unique<TGlobalLog>(
            CreateLogBackend(
                NLoggingImpl::PrepareToOpenLog(logType, logLevel, rotation, startAsDaemon),
                (ELogPriority)logLevel,
                threaded)),
        std::move(formatter));
}

void DoInitGlobalLog(std::unique_ptr<TLogBackend> backend, std::unique_ptr<ILoggerFormatter> formatter) {
    DoInitGlobalLog(std::make_unique<TGlobalLog>(std::move(backend)), std::move(formatter));
}

bool GlobalLogInitialized() {
    return TLoggerOperator<TGlobalLog>::Usage();
}

template <>
TGlobalLog* CreateDefaultLogger<TGlobalLog>() {
    return new TGlobalLog("console", TLOG_INFO);
}

template <>
TNullLog* CreateDefaultLogger<TNullLog>() {
    return new TNullLog("null");
}

NPrivateGlobalLogger::TVerifyEvent::~TVerifyEvent() {
    const std::string info = Str();
    FATAL_LOG << info << Endl;
    Y_ABORT("%s", info.data());
}
