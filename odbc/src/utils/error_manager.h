#pragma once

#include "odbc_compat.h"
#include <vector>
#include <string>
#include <exception>

#include <ydb-cpp-sdk/client/types/status/status.h>

namespace NYdb::NOdbc {

struct TErrorInfo {
    std::string SqlState;
    SQLINTEGER NativeError;
    std::string Message;
};

class TOdbcException : public std::exception {
public:
    TOdbcException(const std::string& sqlState, SQLINTEGER nativeError,
                    const std::string& message, SQLRETURN returnCode = SQL_ERROR)
        : SqlState_(sqlState)
        , NativeError_(nativeError)
        , Message_(message)
        , ReturnCode_(returnCode)
    {}

    const std::string& GetSqlState() const {
        return SqlState_;
    }

    SQLINTEGER GetNativeError() const {
        return NativeError_;
    }

    const std::string& GetMessage() const {
        return Message_;
    }

    SQLRETURN GetReturnCode() const {
        return ReturnCode_;
    }

    const char* what() const noexcept override {
        return Message_.c_str();
    }

private:
    std::string SqlState_;
    SQLINTEGER NativeError_;
    std::string Message_;
    SQLRETURN ReturnCode_;
};

class TErrorManager {
public:
    virtual ~TErrorManager() = default;
    SQLRETURN AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message, SQLRETURN returnCode = SQL_ERROR);
    SQLRETURN AddError(const TOdbcException& ex);
    SQLRETURN AddError(const TStatus& status);

    void ClearErrors();

    void SetLastReturnCode(SQLRETURN code) {
        LastReturnCode_ = code;
    }

    SQLRETURN GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                        SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength);
    virtual SQLRETURN GetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagIdentifier,
                           SQLPOINTER diagInfoPtr, SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthPtr);

private:
    std::vector<TErrorInfo> Errors_;
    SQLRETURN LastReturnCode_ = SQL_SUCCESS;
};

SQLRETURN RecordCurrentException(TErrorManager& errors);

} // namespace NYdb::NOdbc
