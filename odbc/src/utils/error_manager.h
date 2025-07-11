#pragma once

#include <sql.h>
#include <sqlext.h>
#include <vector>
#include <string>

#include <ydb-cpp-sdk/client/types/status/status.h>

namespace NYdb {
namespace NOdbc {

struct TErrorInfo {
    std::string SqlState;
    SQLINTEGER NativeError;
    std::string Message;
    SQLRETURN ReturnCode;
};

using TErrorList = std::vector<TErrorInfo>;

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
    SQLRETURN AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message, SQLRETURN returnCode = SQL_ERROR);
    SQLRETURN AddError(const TOdbcException& ex);
    SQLRETURN AddError(const TStatus& status);

    void ClearErrors();

    SQLRETURN GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                        SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength);

private:
    TErrorList Errors_;
};

template <typename Handle>
SQLRETURN HandleOdbcExceptions(SQLHANDLE handlePtr, std::function<SQLRETURN(Handle*)>&& func) {
    if (!handlePtr) {
        return SQL_INVALID_HANDLE;
    }
    auto handle = static_cast<Handle*>(handlePtr);

    try {
        return func(handle);
    } catch (const NStatusHelpers::TYdbErrorException& ex) {
        return handle->AddError(ex.GetStatus());
    } catch (const TOdbcException& ex) {
        return handle->AddError(ex);
    } catch (const std::exception& ex) {
        return handle->AddError("HY000", 0, ex.what());
    } catch (...) {
        return handle->AddError("HY000", 0, "Unknown error");
    }
}

SQLRETURN HandleOdbcExceptions(SQLHANDLE handlePtr, std::function<SQLRETURN()>&& func);

} // namespace NOdbc
} // namespace NYdb
