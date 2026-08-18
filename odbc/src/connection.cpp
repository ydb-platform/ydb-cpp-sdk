#include "connection.h"
#include "statement.h"

#include <ydb-cpp-sdk/client/result/result.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <string>
#include <algorithm>
#include <cstring>

#include <sql.h>
#include <sqlext.h>

namespace NYdb::NOdbc {

TConnection::~TConnection() {
    DestroyYdbState();
}

void TConnection::DestroyYdbState() {
    QuerySession_.reset();
    Tx_.reset();
    Ydb_.reset();
}

SQLRETURN TConnection::DriverConnect(std::string_view connectionString) {
    std::vector<std::string> ignoredAttributes;
    TConnectionParameters explicitParameters =
        ParseAndNormalizeConnectionString(connectionString, ignoredAttributes);
    const auto dsnIt = explicitParameters.find("DSN");
    TConnectionParameters parameters;
    if (dsnIt != explicitParameters.end() && !dsnIt->second.empty()) {
        parameters = ReadDsnParameters(dsnIt->second);
    }
    OverlayConnectionParameters(parameters, explicitParameters);
    ApplyResolvedSettings(ResolveConnectionSettings(std::move(parameters)));

    if (!ignoredAttributes.empty()) {
        std::string message = ignoredAttributes.size() == 1
            ? "Invalid connection string attribute ignored: "
            : "Invalid connection string attributes ignored: ";
        for (size_t i = 0; i < ignoredAttributes.size(); ++i) {
            if (i != 0) {
                message += ", ";
            }
            message += ignoredAttributes[i];
        }
        return AddError("01S00", 0, message, SQL_SUCCESS_WITH_INFO);
    }

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Connect(std::string_view serverName,
                               std::string_view userName,
                               std::string_view auth) {
    TConnectionParameters parameters = ReadDsnParameters(serverName);
    if (!userName.empty() || !auth.empty()) {
        for (const std::string_view key : {
                "Token", "MetadataHost", "MetadataPort", "ServiceAccountKeyFile",
                "OAuth2KeyFile", "IamEndpoint"}) {
            parameters.erase(std::string(key));
        }
        parameters["AuthMode"] = "Static";
    }
    if (!userName.empty()) {
        parameters["User"] = std::string(userName);
    }
    if (!auth.empty()) {
        parameters["Password"] = std::string(auth);
    }
    ApplyResolvedSettings(ResolveConnectionSettings(std::move(parameters), std::string(serverName)));

    return SQL_SUCCESS;
}

SQLRETURN TConnection::Disconnect() {
    DestroyYdbState();
    DriverConfig_.reset();
    DbmsVersionCache_.reset();
    Database_.clear();
    DataSourceName_.clear();
    return SQL_SUCCESS;
}

NQuery::TSession& TConnection::GetOrCreateQuerySession() {
    if (!QuerySession_) {
        auto sessionResult = Ydb_->QueryClient.GetSession().ExtractValueSync();
        NStatusHelpers::ThrowOnError(sessionResult);
        QuerySession_.emplace(std::move(sessionResult.GetSession()));
    }
    return *QuerySession_;
}

std::optional<NQuery::TQueryClient> TConnection::GetClient() {
    if (!Ydb_) {
        return std::nullopt;
    }
    return Ydb_->QueryClient;
}

std::optional<NTable::TTableClient> TConnection::GetTableClient() {
    if (!Ydb_) {
        return std::nullopt;
    }
    return Ydb_->TableClient;
}

std::optional<NScheme::TSchemeClient> TConnection::GetSchemeClient() {
    if (!Ydb_) {
        return std::nullopt;
    }
    return Ydb_->SchemeClient;
}

std::unique_ptr<TStatement> TConnection::CreateStatement() {
    return std::make_unique<TStatement>(this);
}

void TConnection::CloseStatementCursors() {
    for (TStatement* stmt : Statements_) {
        stmt->Close(true);
    }
}

SQLRETURN TConnection::SetAutocommit(bool value) {
    if (value && Tx_) {
        auto status = Tx_->Commit().ExtractValueSync();
        NStatusHelpers::ThrowOnError(status);
        Tx_.reset();
    }
    return Attributes_.SetAutocommit(value);
}

bool TConnection::GetAutocommit() const {
    return Attributes_.GetAutocommit();
}

SQLRETURN TConnection::SetConnectAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength) {
    if (attr == SQL_ATTR_AUTOCOMMIT) {
        const intptr_t token = ReadIntegerAttr<intptr_t>(value);
        if (token != SQL_AUTOCOMMIT_ON && token != SQL_AUTOCOMMIT_OFF) {
            return Diag::AddInvalidAttrValue(*this, "SQL_ATTR_AUTOCOMMIT");
        }
        return SetAutocommit(token == SQL_AUTOCOMMIT_ON);
    }
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
    return Attributes_.SetConnectAttr(attr, value, *this);
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
        return SQL_SUCCESS;
    }
    auto status = Tx_->Commit().ExtractValueSync();
    NStatusHelpers::ThrowOnError(status);
    Tx_.reset();
    CloseStatementCursors();
    return SQL_SUCCESS;
}

SQLRETURN TConnection::RollbackTx() {
    if (!Tx_) {
        return SQL_SUCCESS;
    }
    auto status = Tx_->Rollback().ExtractValueSync();
    NStatusHelpers::ThrowOnError(status);
    Tx_.reset();
    CloseStatementCursors();
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

    auto client = GetClient();
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
    if (!DriverConfig_) {
        throw TOdbcException("08003", 0, "Connection configuration is not available");
    }
    DestroyYdbState();
    DbmsVersionCache_.reset();
    Ydb_.emplace(*DriverConfig_);
}

void TConnection::ApplyResolvedSettings(TResolvedConnectionSettings&& settings) {
    TConnectionAttributes::NormalizeCatalogPath(settings.Database);
    settings.DriverConfig.SetDatabase(settings.Database);

    Database_ = std::move(settings.Database);
    DataSourceName_ = std::move(settings.DataSourceName);
    DriverConfig_.emplace(std::move(settings.DriverConfig));
    RecreateYdbClients();
    Attributes_.SetCurrentCatalog(Database_);
}

void TConnection::RebindToDatabase(std::string_view newDatabase) {
    if (!DriverConfig_) {
        throw TOdbcException("08003", 0, "Connection configuration is not available");
    }
    std::string db(newDatabase);
    TConnectionAttributes::NormalizeCatalogPath(db);
    Database_ = std::move(db);
    DriverConfig_->SetDatabase(Database_);
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

TConnectionAttributes::TCatalogBinding TConnection::GetCatalogBinding() const {
    return Attributes_.BuildCatalogBinding(Database_);
}

SQLRETURN TConnection::NativeSql(const std::string& inSql, SQLCHAR* outSql, SQLINTEGER outMax, SQLINTEGER* outLen) {
    const SQLINTEGER fullLen = static_cast<SQLINTEGER>(inSql.size());
    if (outLen) {
        *outLen = fullLen;
    }
    if (!outSql) {
        return outMax == 0 ? SQL_SUCCESS : AddError("HY090", 0, "Invalid string or buffer length");
    }
    if (outMax <= 0) {
        return fullLen == 0 ? SQL_SUCCESS : AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
    }
    const SQLINTEGER copyLen = std::min(fullLen, outMax - 1);
    if (copyLen > 0) {
        std::memcpy(outSql, inSql.data(), static_cast<size_t>(copyLen));
    }
    outSql[copyLen] = '\0';
    if (copyLen < fullLen) {
        return AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
    }
    return SQL_SUCCESS;
}

} // namespace NYdb::NOdbc
