#include "error_manager.h"
#include "diag.h"

#include <string>

namespace NYdb::NOdbc {

namespace {
    struct OdbcErrorMapping {
        const char* sqlState;
        const char* description;
        SQLRETURN returnCode;
    };

    OdbcErrorMapping GetErrorMappingForStatus(EStatus status) {
#define ODBC_STATUS(name, state, text, result) case EStatus::name: return {state, text, result}
        switch (status) {
            ODBC_STATUS(SUCCESS, "00000", "Success", SQL_SUCCESS);
            ODBC_STATUS(BAD_REQUEST, "42000", "Syntax error or access rule violation", SQL_ERROR);
            ODBC_STATUS(UNAUTHORIZED, "28000", "Invalid authorization specification", SQL_ERROR);
            ODBC_STATUS(INTERNAL_ERROR, "HY000", "General error", SQL_ERROR);
            ODBC_STATUS(ABORTED, "40001", "Serialization failure", SQL_ERROR);
            ODBC_STATUS(UNAVAILABLE, "08S01", "Communication link failure", SQL_ERROR);
            ODBC_STATUS(OVERLOADED, "HY000", "General error - server overloaded", SQL_ERROR);
            ODBC_STATUS(SCHEME_ERROR, "42S02", "Base table or view not found", SQL_ERROR);
            ODBC_STATUS(GENERIC_ERROR, "HY000", "General error", SQL_ERROR);
            ODBC_STATUS(TIMEOUT, "HYT00", "Timeout expired", SQL_ERROR);
            ODBC_STATUS(BAD_SESSION, "08003", "Connection does not exist", SQL_ERROR);
            ODBC_STATUS(PRECONDITION_FAILED, "23000", "Integrity constraint violation", SQL_ERROR);
            ODBC_STATUS(ALREADY_EXISTS, "23000", "Integrity constraint violation", SQL_ERROR);
            ODBC_STATUS(NOT_FOUND, "02000", "No data found", SQL_NO_DATA);
            ODBC_STATUS(SESSION_EXPIRED, "08003", "Connection does not exist", SQL_ERROR);
            ODBC_STATUS(CANCELLED, "HY008", "Operation canceled", SQL_ERROR);
            ODBC_STATUS(UNDETERMINED, "40003", "Statement completion unknown", SQL_ERROR);
            ODBC_STATUS(UNSUPPORTED, "HYC00", "Optional feature not implemented", SQL_ERROR);
            ODBC_STATUS(SESSION_BUSY, "HY000", "General error - session busy", SQL_ERROR);
            ODBC_STATUS(TRANSPORT_UNAVAILABLE, "08S01", "Communication link failure", SQL_ERROR);
            ODBC_STATUS(CLIENT_RESOURCE_EXHAUSTED, "HY000", "General error - resource exhausted", SQL_ERROR);
            ODBC_STATUS(CLIENT_DEADLINE_EXCEEDED, "HYT00", "Timeout expired", SQL_ERROR);
            ODBC_STATUS(CLIENT_INTERNAL_ERROR, "HY000", "General error", SQL_ERROR);
            ODBC_STATUS(CLIENT_CANCELLED, "HY008", "Operation canceled", SQL_ERROR);
            ODBC_STATUS(CLIENT_UNAUTHENTICATED, "28000", "Invalid authorization specification", SQL_ERROR);
            ODBC_STATUS(CLIENT_LIMITS_REACHED, "HY000", "General error - limits reached", SQL_ERROR);
            ODBC_STATUS(CLIENT_DISCOVERY_FAILED, "08001", "Client unable to establish connection", SQL_ERROR);
            ODBC_STATUS(CLIENT_CALL_UNIMPLEMENTED, "HYC00", "Optional feature not implemented", SQL_ERROR);
            ODBC_STATUS(CLIENT_OUT_OF_RANGE, "22003", "Numeric value out of range", SQL_ERROR);
            default: return {"HY000", "Unknown YDB error", SQL_ERROR};
        }
#undef ODBC_STATUS
    }

    SQLRETURN WriteDiagCStr(std::string_view value, SQLPOINTER output, SQLSMALLINT bufferLength,
                            SQLSMALLINT* length, bool sqlState = false) {
        std::string normalized;
        if (sqlState) {
            normalized = value;
            normalized.resize(5, ' ');
            value = normalized;
        }
        return Diag::WriteString<Diag::EStringWriteMode::Diagnostic>(
            nullptr, value, output, bufferLength, length);
    }

} // namespace

SQLRETURN RecordCurrentException(TErrorManager& errors) {
    try {
        throw;
    } catch (const NStatusHelpers::TYdbErrorException& ex) {
        return errors.AddError(ex.GetStatus());
    } catch (const TOdbcException& ex) {
        return errors.AddError(ex);
    } catch (const std::exception& ex) {
        return errors.AddError("HY000", 0, ex.what());
    } catch (...) {
        return errors.AddError("HY000", 0, "Unknown error");
    }
}

SQLRETURN TErrorManager::AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message, SQLRETURN returnCode) {
    Errors_.push_back({sqlState, nativeError, message});
    LastReturnCode_ = returnCode;
    return returnCode;
}

SQLRETURN TErrorManager::AddError(const TOdbcException& ex) {
    Errors_.push_back({ex.GetSqlState(), ex.GetNativeError(), ex.GetMessage()});
    LastReturnCode_ = ex.GetReturnCode();
    return ex.GetReturnCode();
}

SQLRETURN TErrorManager::AddError(const TStatus& status) {
    auto mapping = GetErrorMappingForStatus(status.GetStatus());
    std::string message = mapping.description;
    if (!status.GetIssues().Empty()) {
        message += ": " + status.GetIssues().ToString();
    }
    Errors_.push_back({mapping.sqlState, static_cast<SQLINTEGER>(status.GetStatus()), message});
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
        WriteDiagCStr(err.SqlState, sqlState, 6, nullptr, true);
    }

    if (nativeError) {
        *nativeError = err.NativeError;
    }

    return WriteDiagCStr(err.Message, messageText, bufferLength, textLength, false);
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

} // namespace NYdb::NOdbc
