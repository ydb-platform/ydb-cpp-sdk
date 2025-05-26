#pragma once

#include <sql.h>
#include <sqlext.h>

#include <vector>
#include <string>

namespace NYdb {
namespace NOdbc {

class TConnection;

struct TErrorInfo {
    std::string SqlState;
    SQLINTEGER NativeError;
    std::string Message;
};

using TErrorList = std::vector<TErrorInfo>;

class TEnvironment {
private:
    SQLINTEGER OdbcVersion_;
    TErrorList Errors_;

public:
    TEnvironment();
    ~TEnvironment();

    SQLRETURN SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                        SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength);

    void AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message);
    void ClearErrors();
};

} // namespace NOdbc
} // namespace NYdb
