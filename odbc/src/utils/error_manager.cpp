#include "error_manager.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <unordered_map>

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

namespace {

SQLRETURN WriteDiagCStr(
    const std::string& str,
    SQLPOINTER diagInfoPtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthPtr,
    bool sqlStateField = false) {
    std::string storage;
    const std::string* src = &str;
    if (sqlStateField) {
        storage = str;
        if (storage.size() < 5) {
            storage.append(5U - storage.size(), ' ');
        } else {
            storage.resize(5U);
        }
        src = &storage;
    }
    if (!diagInfoPtr) {
        return SQL_ERROR;
    }
    if (bufferLength < 0) {
        return SQL_ERROR;
    }
    const size_t fullLen = src->size();
    if (stringLengthPtr) {
        *stringLengthPtr = static_cast<SQLSMALLINT>(std::min<size_t>(fullLen, 0x7FFFU));
    }
    if (bufferLength == 0) {
        return fullLen == 0 ? SQL_SUCCESS : SQL_SUCCESS_WITH_INFO;
    }
    auto* out = static_cast<SQLCHAR*>(diagInfoPtr);
    const size_t maxData = static_cast<size_t>(bufferLength - 1U);
    const size_t copyLen = std::min(fullLen, maxData);
    std::memcpy(out, src->data(), copyLen);
    out[copyLen] = 0;
    return (fullLen > maxData) ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;
}

} // namespace

SQLRETURN TErrorManager::AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message, SQLRETURN returnCode) {
    Errors_.push_back({sqlState, nativeError, message, returnCode});
    LastReturnCode_ = returnCode;
    return returnCode;
}

SQLRETURN TErrorManager::AddError(const TOdbcException& ex) {
    Errors_.push_back({ex.GetSqlState(), ex.GetNativeError(), ex.GetMessage(), ex.GetReturnCode()});
    LastReturnCode_ = ex.GetReturnCode();
    return ex.GetReturnCode();
}

SQLRETURN TErrorManager::AddError(const TStatus& status) {
    auto mapping = GetErrorMappingForStatus(status.GetStatus());
    std::string message = mapping.description;
    if (!status.GetIssues().Empty()) {
        message += ": " + status.GetIssues().ToString();
    }
    Errors_.push_back({mapping.sqlState, static_cast<SQLINTEGER>(status.GetStatus()), message, mapping.returnCode});
    LastReturnCode_ = mapping.returnCode;
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

SQLRETURN TErrorManager::GetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagIdentifier, SQLPOINTER diagInfoPtr,
    SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthPtr) {
    const SQLSMALLINT count = static_cast<SQLSMALLINT>(Errors_.size());
    if (diagInfoPtr == nullptr) {
        return SQL_ERROR;
    }
    if (recNumber == 0) {
        switch (diagIdentifier) {
            case SQL_DIAG_RETURNCODE:
                *static_cast<SQLRETURN*>(diagInfoPtr) = LastReturnCode_;
                return SQL_SUCCESS;
            case SQL_DIAG_NUMBER: {
                *static_cast<SQLINTEGER*>(diagInfoPtr) = static_cast<SQLINTEGER>(count);
                return SQL_SUCCESS;
            }
            case SQL_DIAG_ROW_COUNT:
                return SQL_ERROR;
            default:
                return SQL_ERROR;
        }
    }

    if (recNumber < 1 || recNumber > count) {
        return SQL_NO_DATA;
    }

    const auto& err = Errors_[recNumber - 1];
    switch (diagIdentifier) {
        case SQL_DIAG_SQLSTATE:
            return WriteDiagCStr(err.SqlState, diagInfoPtr, bufferLength, stringLengthPtr, true);
        case SQL_DIAG_NATIVE: {
            *static_cast<SQLINTEGER*>(diagInfoPtr) = err.NativeError;
            return SQL_SUCCESS;
        }
        case SQL_DIAG_MESSAGE_TEXT:
            return WriteDiagCStr(err.Message, diagInfoPtr, bufferLength, stringLengthPtr);
        case SQL_DIAG_CLASS_ORIGIN:
            return WriteDiagCStr("ODBC 3.0", diagInfoPtr, bufferLength, stringLengthPtr);
        case SQL_DIAG_SUBCLASS_ORIGIN:
            return WriteDiagCStr("ODBC 3.0", diagInfoPtr, bufferLength, stringLengthPtr);
        case SQL_DIAG_CONNECTION_NAME:
        case SQL_DIAG_SERVER_NAME:
            return WriteDiagCStr("", diagInfoPtr, bufferLength, stringLengthPtr);
        case SQL_DIAG_COLUMN_NUMBER:
            *static_cast<SQLINTEGER*>(diagInfoPtr) = SQL_COLUMN_NUMBER_UNKNOWN;
            return SQL_SUCCESS;
        case SQL_DIAG_ROW_NUMBER:
            *static_cast<SQLLEN*>(diagInfoPtr) = SQL_ROW_NUMBER_UNKNOWN;
            return SQL_SUCCESS;
        default:
            return SQL_ERROR;
    }
}

SQLRETURN HandleOdbcExceptions(
    SQLHANDLE handlePtr,
    std::function<SQLRETURN()>&& func,
    ENullInputHandlePolicy nullInputPolicy) {
    if (!handlePtr && nullInputPolicy != ENullInputHandlePolicy::Allow) {
        return SQL_INVALID_HANDLE;
    }

    try {
        const SQLRETURN r = func();
        if (handlePtr) {
            static_cast<TErrorManager*>(handlePtr)->SetLastReturnCode(r);
        }
        return r;
    } catch (...) {
        if (handlePtr) {
            static_cast<TErrorManager*>(handlePtr)->SetLastReturnCode(SQL_ERROR);
        }
        return SQL_ERROR;
    }
}

} // namespace NOdbc
} // namespace NYdb 