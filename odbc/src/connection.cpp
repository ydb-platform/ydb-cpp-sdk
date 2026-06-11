#include "connection.h"
#include "statement.h"
#include "utils/error_manager.h"

#include <ydb-cpp-sdk/client/result/result.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <map>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <sql.h>
#include <sqlext.h>

#include <odbcinst.h>

namespace NYdb {
namespace NOdbc {

namespace {

struct TDriverKey {
    std::string Endpoint;
    std::string Database;

    bool operator==(const TDriverKey& other) const noexcept {
        return Endpoint == other.Endpoint && Database == other.Database;
    }
};

struct TDriverKeyHash {
    size_t operator()(const TDriverKey& key) const noexcept {
        return std::hash<std::string>{}(key.Endpoint) ^ (std::hash<std::string>{}(key.Database) << 1U);
    }
};

struct TDriverPool {
    std::unordered_map<TDriverKey, std::weak_ptr<NYdb::TDriver>, TDriverKeyHash> DriversByKey;
    size_t InsertionsSinceCleanup = 0;
};

void CleanupExpiredDrivers(TDriverPool& pool) {
    for (auto mapIt = pool.DriversByKey.begin(); mapIt != pool.DriversByKey.end();) {
        if (mapIt->second.expired()) {
            mapIt = pool.DriversByKey.erase(mapIt);
        } else {
            ++mapIt;
        }
    }
}

std::shared_ptr<NYdb::TDriver> AcquireSharedDriver(const std::string& endpoint, const std::string& database) {
    static TDriverPool pool;
    TDriverKey key{endpoint, database};
    auto it = pool.DriversByKey.find(key);
    if (it != pool.DriversByKey.end()) {
        if (std::shared_ptr<NYdb::TDriver> existing = it->second.lock()) {
            return existing;
        }
    }
    auto driver = std::make_shared<NYdb::TDriver>(
        NYdb::TDriverConfig().SetEndpoint(endpoint).SetDatabase(database));
    pool.DriversByKey[std::move(key)] = driver;
    ++pool.InsertionsSinceCleanup;
    if (pool.InsertionsSinceCleanup >= 32) {
        CleanupExpiredDrivers(pool);
        pool.InsertionsSinceCleanup = 0;
    }
    return driver;
}

} // namespace

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
    Endpoint_ = params.contains("Server") ? params["Server"] : params["Endpoint"];
    Database_ = params["Database"];
    DataSourceName_ = params.contains("DSN") ? params["DSN"] : "";

    if (Endpoint_.empty() || Database_.empty()) {
        throw TOdbcException("08001", 0, "Missing Endpoint (or Server) or Database in connection string");
    }

    TConnectionAttributes::NormalizeCatalogPath(Database_);
    RecreateYdbClients();
    Attributes_.SetCurrentCatalog(Database_);

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Connect(const std::string& serverName,
                               const std::string& userName,
                               const std::string& auth) {
    DataSourceName_ = serverName;

    char endpoint[256] = {0};
    char server[256] = {0};
    char database[256] = {0};

    SQLGetPrivateProfileString(serverName.c_str(), "Endpoint", "", endpoint, sizeof(endpoint), nullptr);
    SQLGetPrivateProfileString(serverName.c_str(), "Server", "", server, sizeof(server), nullptr);
    SQLGetPrivateProfileString(serverName.c_str(), "Database", "", database, sizeof(database), nullptr);

    Endpoint_ = endpoint[0] ? endpoint : server;
    Database_ = database;

    if (Endpoint_.empty() || Database_.empty()) {
        throw TOdbcException("08001", 0, "Missing Endpoint (or Server) or Database in DSN");
    }

    TConnectionAttributes::NormalizeCatalogPath(Database_);
    RecreateYdbClients();
    Attributes_.SetCurrentCatalog(Database_);

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Disconnect() {
    QuerySession_.reset();
    Tx_.reset();
    DbmsVersionCache_.reset();
    DataSourceName_.clear();
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
    if (attr == SQL_ATTR_CURRENT_CATALOG) {
        std::optional<std::string> rebindDatabase;
        SQLRETURN rc = Attributes_.ApplyCatalogChange(value, stringLength, Database_, rebindDatabase, *this);
        if (rc != SQL_SUCCESS) {
            return rc;
        }
        if (rebindDatabase) {
            RebindToDatabase(*rebindDatabase);
        }
        return SQL_SUCCESS;
    }
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

void TConnection::ResetTx() {
    Tx_.reset();
}

void TConnection::ResetQuerySession() {
    QuerySession_.reset();
}

SQLRETURN TConnection::CommitTx() {
    if (!Tx_) {
        return AddError("25000", 0, "Invalid transaction state: no active transaction");
    }
    auto status = Tx_->Commit().ExtractValueSync();
    NStatusHelpers::ThrowOnError(status);
    Tx_.reset();
    return SQL_SUCCESS;
}

SQLRETURN TConnection::RollbackTx() {
    if (!Tx_) {
        return AddError("25000", 0, "Invalid transaction state: no active transaction");
    }
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

const std::string& TConnection::GetDataSourceName() const {
    return DataSourceName_;
}

SQLUINTEGER TConnection::GetSupportedTxnIsolationOptions() const {
    return Attributes_.GetSupportedTxnIsolationOptions();
}

bool TConnection::IsDataSourceReadOnly() const {
    return Attributes_.GetAccessMode() == SQL_MODE_READ_ONLY;
}

const std::string& TConnection::GetDbmsVersion() {
    if (DbmsVersionCache_) {
        return *DbmsVersionCache_;
    }

    auto* client = GetClient();
    if (!client) {
        throw TOdbcException("08003", 0, "Connection is not established");
    }

    std::optional<std::string> fetched;
    const NYdb::TStatus status = client->RetryQuerySync(
        [&fetched](NQuery::TSession session) -> NYdb::TStatus {
            auto result = session.ExecuteQuery(
                "SELECT Version();",
                NQuery::TTxControl::NoTx(),
                NYdb::TParamsBuilder().Build()).ExtractValueSync();
            if (!result.IsSuccess()) {
                return result;
            }
            if (result.GetResultSets().empty()) {
                return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            }
            TResultSetParser parser(result.GetResultSetParser(0));
            if (parser.TryNextRow()) {
                fetched = parser.ColumnParser(0).GetUtf8();
            }
            return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
        });

    NStatusHelpers::ThrowOnError(status);
    if (!fetched || fetched->empty()) {
        throw TOdbcException("HY000", 0, "Failed to retrieve DBMS version");
    }

    DbmsVersionCache_ = std::move(*fetched);
    return *DbmsVersionCache_;
}

void TConnection::RecreateYdbClients() {
    QuerySession_.reset();
    Tx_.reset();
    DbmsVersionCache_.reset();
    YdbSchemeClient_.reset();
    YdbTableClient_.reset();
    YdbClient_.reset();
    YdbDriver_ = AcquireSharedDriver(Endpoint_, Database_);
    YdbClient_ = std::make_unique<NYdb::NQuery::TQueryClient>(*YdbDriver_);
    YdbSchemeClient_ = std::make_unique<NYdb::NScheme::TSchemeClient>(*YdbDriver_);
    YdbTableClient_ = std::make_unique<NYdb::NTable::TTableClient>(*YdbDriver_);
}

void TConnection::RebindToDatabase(const std::string& newDatabase) {
    std::string db = newDatabase;
    TConnectionAttributes::NormalizeCatalogPath(db);
    Database_ = std::move(db);
    Attributes_.SetCurrentCatalog(Database_);
    RecreateYdbClients();
}


std::string TConnection::WrapQueryForCurrentCatalog(const std::string& sql) const {
    std::optional<std::string> rel = Attributes_.ResolveCatalogRoute(Database_).TablePathPrefix;
    if (!rel) {
        return sql;
    }
    std::string escapedPrefix;
    escapedPrefix.reserve(rel->size() + 8);
    for (const char ch : *rel) {
        if (ch == '\\' || ch == '"') {
            escapedPrefix.push_back('\\');
        }
        escapedPrefix.push_back(ch);
    }
    return "PRAGMA TablePathPrefix = \"" + escapedPrefix + "\";\n" + sql;
}
} // namespace NOdbc
} // namespace NYdb
