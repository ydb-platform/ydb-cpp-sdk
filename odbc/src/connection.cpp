#include "connection.h"
#include "statement.h"
#include "utils/error_manager.h"

#include <map>
#include <string>

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
        throw TOdbcException("08001", 0, "Missing Endpoint or Database in connection string");
    }

    YdbDriver_ = std::make_unique<NYdb::TDriver>(NYdb::TDriverConfig()
        .SetEndpoint(Endpoint_)
        .SetDatabase(Database_));

    YdbClient_ = std::make_unique<NYdb::NQuery::TQueryClient>(*YdbDriver_);
    YdbSchemeClient_ = std::make_unique<NYdb::NScheme::TSchemeClient>(*YdbDriver_);
    YdbTableClient_ = std::make_unique<NYdb::NTable::TTableClient>(*YdbDriver_);

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Connect(const std::string& serverName,
                               const std::string& userName,
                               const std::string& auth) {

    char endpoint[256] = {0};
    char database[256] = {0};

    //SQLGetPrivateProfileString(serverName.c_str(), "Endpoint", "", endpoint, sizeof(endpoint), nullptr);
    //SQLGetPrivateProfileString(serverName.c_str(), "Database", "", database, sizeof(database), nullptr);

    Endpoint_ = endpoint;
    Database_ = database;

    if (Endpoint_.empty() || Database_.empty()) {
        throw TOdbcException("08001", 0, "Missing Endpoint or Database in DSN");
    }

    YdbDriver_ = std::make_unique<NYdb::TDriver>(NYdb::TDriverConfig()
        .SetEndpoint(Endpoint_)
        .SetDatabase(Database_));

    YdbClient_ = std::make_unique<NYdb::NQuery::TQueryClient>(*YdbDriver_);
    YdbSchemeClient_ = std::make_unique<NYdb::NScheme::TSchemeClient>(*YdbDriver_);
    YdbTableClient_ = std::make_unique<NYdb::NTable::TTableClient>(*YdbDriver_);

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Disconnect() {
    QuerySession_.reset();
    Tx_.reset();
    YdbSchemeClient_.reset();
    YdbTableClient_.reset();
    YdbClient_.reset();
    YdbDriver_.reset();
    return SQL_SUCCESS;
}

NQuery::TSession& TConnection::GetOrCreateQuerySession() {
    if (!QuerySession_) {
        auto sessionResult = YdbClient_->GetSession().ExtractValueSync();
        NStatusHelpers::ThrowOnError(sessionResult);
        QuerySession_.emplace(std::move(sessionResult.GetSession()));
    }
    return *QuerySession_;
}

std::unique_ptr<TStatement> TConnection::CreateStatement() {
    return std::make_unique<TStatement>(this);
}

void TConnection::RemoveStatement(TStatement* stmt) {
    Statements_.erase(std::remove_if(Statements_.begin(), Statements_.end(),
        [stmt](const std::unique_ptr<TStatement>& s) { return s.get() == stmt; }), Statements_.end());
}

SQLRETURN TConnection::SetAutocommit(bool value) {
    Attributes_.SetAutocommit(value);
    if (Attributes_.GetAutocommit() && Tx_) {
        auto status = Tx_->Commit().ExtractValueSync();
        NStatusHelpers::ThrowOnError(status);
        Tx_.reset();
    }
    return SQL_SUCCESS;
}

bool TConnection::GetAutocommit() const {
    return Attributes_.GetAutocommit();
}

SQLRETURN TConnection::SetConnectAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength) {
    return Attributes_.SetConnectAttr(attr, value, stringLength, [this](bool autocommit) {
        return SetAutocommit(autocommit);
    }, *this);
}

SQLRETURN TConnection::GetConnectAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength,
                                      SQLINTEGER* stringLengthPtr) {
    return Attributes_.GetConnectAttr(attr, value, bufferLength, stringLengthPtr, *this);
}

NQuery::TTxSettings TConnection::MakeTxSettings() const {
    return Attributes_.MakeTxSettings();
}

const std::optional<NQuery::TTransaction>& TConnection::GetTx() {
    return Tx_;
}

void TConnection::SetTx(const NQuery::TTransaction& tx) {
    Tx_ = tx;
}

void TConnection::Reset() {
    Tx_.reset();
}

SQLRETURN TConnection::CommitTx() {
    auto status = Tx_->Commit().ExtractValueSync();
    NStatusHelpers::ThrowOnError(status);
    Tx_.reset();
    return SQL_SUCCESS;
}

SQLRETURN TConnection::RollbackTx() {
    auto status = Tx_->Rollback().ExtractValueSync();
    NStatusHelpers::ThrowOnError(status);
    Tx_.reset();
    return SQL_SUCCESS;
}

void TConnection::SetEnvironment(TEnvironment* env){
    if (ParentEnv_){
        throw std::logic_error("Connection already bound to environment");
    }
    ParentEnv_ = env;
}

TEnvironment* TConnection::GetEnvironment(){
    return ParentEnv_;
}

} // namespace NOdbc
} // namespace NYdb
