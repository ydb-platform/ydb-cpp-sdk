#include "error_manager.h"

#include <algorithm>
#include <cstring>

namespace NYdb {
namespace NOdbc {
namespace {
    struct OdbcErrorMapping {
        const char* sqlState;
        const char* description;
        SQLRETURN returnCode;
    };

    const std::unordered_map<EStatus, OdbcErrorMapping> ERROR_MAPPINGS = {
        {EStatus::SUCCESS,              {"00000", "Success", SQL_SUCCESS}},
        {EStatus::BAD_REQUEST,          {"42000", "Syntax error or access rule violation", SQL_ERROR}},
        {EStatus::UNAUTHORIZED,         {"28000", "Invalid authorization specification", SQL_ERROR}},
        {EStatus::INTERNAL_ERROR,       {"HY000", "General error", SQL_ERROR}},
        {EStatus::ABORTED,              {"25000", "Invalid transaction state", SQL_ERROR}},
        {EStatus::UNAVAILABLE,          {"08001", "Client unable to establish connection", SQL_ERROR}},
        {EStatus::OVERLOADED,           {"HY000", "General error - server overloaded", SQL_ERROR}},
        {EStatus::SCHEME_ERROR,         {"42S02", "Base table or view not found", SQL_ERROR}},
        {EStatus::GENERIC_ERROR,        {"HY000", "General error", SQL_ERROR}},
        {EStatus::TIMEOUT,              {"HYT00", "Timeout expired", SQL_ERROR}},
        {EStatus::BAD_SESSION,          {"08003", "Connection does not exist", SQL_ERROR}},
        {EStatus::PRECONDITION_FAILED,  {"23000", "Integrity constraint violation", SQL_ERROR}},
        {EStatus::ALREADY_EXISTS,       {"23000", "Integrity constraint violation", SQL_ERROR}},
        {EStatus::NOT_FOUND,            {"02000", "No data found", SQL_NO_DATA}},
        {EStatus::SESSION_EXPIRED,      {"08003", "Connection does not exist", SQL_ERROR}},
        {EStatus::CANCELLED,            {"HY008", "Operation canceled", SQL_ERROR}},
        {EStatus::UNDETERMINED,         {"HY000", "General error", SQL_ERROR}},
        {EStatus::UNSUPPORTED,          {"HYC00", "Optional feature not implemented", SQL_ERROR}},
        {EStatus::SESSION_BUSY,         {"HY000", "General error - session busy", SQL_ERROR}},
        // Transport errors
        {EStatus::TRANSPORT_UNAVAILABLE, {"08001", "Client unable to establish connection", SQL_ERROR}},
        {EStatus::CLIENT_RESOURCE_EXHAUSTED, {"HY000", "General error - resource exhausted", SQL_ERROR}},
        {EStatus::CLIENT_DEADLINE_EXCEEDED, {"HYT00", "Timeout expired", SQL_ERROR}},
        {EStatus::CLIENT_INTERNAL_ERROR, {"HY000", "General error", SQL_ERROR}},
        {EStatus::CLIENT_CANCELLED,     {"HY008", "Operation canceled", SQL_ERROR}},
        {EStatus::CLIENT_UNAUTHENTICATED, {"28000", "Invalid authorization specification", SQL_ERROR}},
        {EStatus::CLIENT_LIMITS_REACHED, {"HY000", "General error - limits reached", SQL_ERROR}},
        {EStatus::CLIENT_DISCOVERY_FAILED, {"08001", "Client unable to establish connection", SQL_ERROR}},
        {EStatus::CLIENT_CALL_UNIMPLEMENTED, {"HYC00", "Optional feature not implemented", SQL_ERROR}},
        {EStatus::CLIENT_OUT_OF_RANGE,  {"22003", "Numeric value out of range", SQL_ERROR}},
    };

    const OdbcErrorMapping DEFAULT_ERROR_MAPPING = {"HY000", "Unknown YDB error", SQL_ERROR};

    OdbcErrorMapping GetErrorMappingForStatus(EStatus status) {
        auto it = ERROR_MAPPINGS.find(status);
        if (it != ERROR_MAPPINGS.end()) {
            return it->second;
        }
        return DEFAULT_ERROR_MAPPING;
    }
} // namespace

SQLRETURN TErrorManager::AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message, SQLRETURN returnCode) {
    Errors_.push_back({sqlState, nativeError, message, returnCode});
    return returnCode;
}

SQLRETURN TErrorManager::AddError(const TOdbcException& ex) {
    Errors_.push_back({ex.GetSqlState(), ex.GetNativeError(), ex.GetMessage(), ex.GetReturnCode()});
    return ex.GetReturnCode();
}

SQLRETURN TErrorManager::AddError(const TStatus& status) {
    auto mapping = GetErrorMappingForStatus(status.GetStatus());
    std::string message = mapping.description;
    if (!status.GetIssues().Empty()) {
        message += ": " + status.GetIssues().ToString();
    }
    Errors_.push_back({mapping.sqlState, static_cast<SQLINTEGER>(status.GetStatus()), message, mapping.returnCode});
    return mapping.returnCode;
}

void TErrorManager::ClearErrors() {
    Errors_.clear();
}

SQLRETURN TErrorManager::GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                                   SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength) {
    if (recNumber < 1 || recNumber > (SQLSMALLINT)Errors_.size()) {
        return SQL_NO_DATA;
    }

    const auto& err = Errors_[recNumber-1];
    if (sqlState) {
        strncpy((char*)sqlState, err.SqlState.c_str(), 6);
    }

    if (nativeError) {
        *nativeError = err.NativeError;
    }

    if (messageText && bufferLength > 0) {
        strncpy((char*)messageText, err.Message.c_str(), bufferLength);
        if (textLength) {
            *textLength = (SQLSMALLINT)std::min((int)err.Message.size(), (int)bufferLength);
        }
    }
    return SQL_SUCCESS;
}

SQLRETURN HandleOdbcExceptions(SQLHANDLE handlePtr, std::function<SQLRETURN()>&& func) {
    if (!handlePtr) {
        return SQL_INVALID_HANDLE;
    }

    try {
        return func();
    } catch (...) {
        return SQL_ERROR;
    }
}

} // namespace NOdbc
} // namespace NYdb 