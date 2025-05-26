#include "connection.h"
#include "statement.h"

#include <cstring>
#include <string>
#include <map>

#include <sql.h>
#include <sqlext.h>

#include <odbcinst.h>

namespace NYdb {
namespace NOdbc {

SQLRETURN TConnection::DriverConnect(const std::string& connectionString) {
    std::map<std::string, std::string> params;
    size_t pos = 0;
    while (pos < connectionString.size()) {
        size_t eq = connectionString.find('=', pos);
        if (eq == std::string::npos) {
            break;
        }

        size_t sc = connectionString.find(';', eq);
        std::string key = connectionString.substr(pos, eq-pos);
        std::string val = connectionString.substr(eq+1, (sc == std::string::npos ? std::string::npos : sc-eq-1));
        params[key] = val;
        if (sc == std::string::npos) {
            break;
        }
        pos = sc+1;
    }
    Endpoint_ = params["Endpoint"];
    Database_ = params["Database"];

    if (Endpoint_.empty() || Database_.empty()) {
        AddError("08001", 0, "Missing Endpoint or Database in connection string");
        return SQL_ERROR;
    }

    YdbDriver_ = std::make_unique<NYdb::TDriver>(NYdb::TDriverConfig()
        .SetEndpoint(Endpoint_)
        .SetDatabase(Database_));

    YdbClient_ = std::make_unique<NYdb::NQuery::TQueryClient>(*YdbDriver_);

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Connect(const std::string& serverName,
                               const std::string& userName,
                               const std::string& auth) {

    char endpoint[256] = {0};
    char database[256] = {0};

    SQLGetPrivateProfileString(serverName.c_str(), "Endpoint", "", endpoint, sizeof(endpoint), nullptr);
    SQLGetPrivateProfileString(serverName.c_str(), "Database", "", database, sizeof(database), nullptr);

    Endpoint_ = endpoint;
    Database_ = database;

    if (Endpoint_.empty() || Database_.empty()) {
        AddError("08001", 0, "Missing Endpoint or Database in DSN");
        return SQL_ERROR;
    }

    YdbDriver_ = std::make_unique<NYdb::TDriver>(NYdb::TDriverConfig()
        .SetEndpoint(Endpoint_)
        .SetDatabase(Database_));

    YdbClient_ = std::make_unique<NYdb::NQuery::TQueryClient>(*YdbDriver_);

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Disconnect() {
    YdbClient_.reset();
    YdbDriver_.reset();
    return SQL_SUCCESS;
}

SQLRETURN TConnection::GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
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

std::unique_ptr<TStatement> TConnection::CreateStatement() {
    return std::make_unique<TStatement>(this);
}

void TConnection::RemoveStatement(TStatement* stmt) {
    Statements_.erase(std::remove_if(Statements_.begin(), Statements_.end(),
        [stmt](const std::unique_ptr<TStatement>& s) { return s.get() == stmt; }), Statements_.end());
}

void TConnection::AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message) {
    Errors_.push_back({sqlState, nativeError, message});
}

void TConnection::ClearErrors() {
    Errors_.clear();
}

std::pair<std::string, std::string> TConnection::ParseConnectionString(const std::string& connectionString) {
    // TODO: Implement
    return {"", ""};
}

} // namespace NOdbc
} // namespace NYdb
