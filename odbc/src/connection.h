#pragma once

#include <sql.h>
#include <sqlext.h>
#include <memory>
#include <vector>
#include <string>

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/query/client.h>

#include "environment.h"

namespace NYdb {
namespace NOdbc {

class TStatement;

class TConnection {
private:
    std::unique_ptr<NYdb::TDriver> YdbDriver_;
    std::unique_ptr<NYdb::NQuery::TQueryClient> YdbClient_;

    TErrorList Errors_;
    std::vector<std::unique_ptr<TStatement>> Statements_;
    std::string Endpoint_;
    std::string Database_;
    std::string AuthToken_;

public:
    SQLRETURN Connect(const std::string& serverName,
                      const std::string& userName,
                      const std::string& auth);

    SQLRETURN DriverConnect(const std::string& connectionString);
    SQLRETURN Disconnect();
    SQLRETURN GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                        SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength);

    std::unique_ptr<TStatement> CreateStatement();
    void RemoveStatement(TStatement* stmt);

    NYdb::NQuery::TQueryClient* GetClient() { return YdbClient_.get(); }

    void AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message);
    void ClearErrors();

private:
    std::pair<std::string, std::string> ParseConnectionString(const std::string& connectionString);
};

} // namespace NOdbc
} // namespace NYdb
