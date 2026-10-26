#include "connection.h"
#include "statement.h"

#include "utils/escape.h"

#include <ydb-cpp-sdk/client/result/result.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <util/generic/scope.h>

#include <string>
#include <cstring>

#include "odbc_compat.h"

namespace NYdb::NOdbc {

void TConnection::DestroyYdbState() {
    Tx_.reset();
    Ydb_.reset();
}

SQLRETURN TConnection::DriverConnect(std::string_view connectionString,
                                     SQLCHAR* outConnectionString,
                                     SQLSMALLINT bufferLength,
                                     SQLSMALLINT* stringLength2Ptr) {
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

    const SQLRETURN outputResult = Diag::WriteOptionalOdbcString(
        *this, connectionString, outConnectionString, bufferLength, stringLength2Ptr);
    if (outputResult == SQL_ERROR) {
        return outputResult;
    }

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

    return outputResult;
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
    if (!IsConnected()) {
        return AddError("08003", 0, "Connection not open");
    }
    for (auto* child : GetChildren()) {
        if (const auto* statement = dynamic_cast<TStatement*>(child); statement && statement->NeedsData()) {
            return AddError("HY010", 0, "Statement still needs parameter data");
        }
    }
    if (Tx_) {
        return AddError("25000", 0, "Transaction must be committed or rolled back before disconnecting");
    }
    // The exclusive lifecycle gate has drained all child calls.
    for (auto* child : ReleaseChildren()) {
        delete child;
    }
    DestroyYdbState();
    DriverConfig_.reset();
    DbmsVersionCache_.reset();
    Database_.clear();
    ServerName_.clear();
    DataSourceName_.clear();
    return SQL_SUCCESS;
}

NQuery::TTransaction& TConnection::GetOrCreateTx(TStatement& statement) {
    if (!Tx_) {
        // Retry only transaction creation; never replay statements in an existing transaction.
        auto status = Ydb_->QueryClient.RetryQuerySync([&](NQuery::TSession session) {
            statement.CheckCancellation();
            auto result = session.BeginTransaction(MakeTxSettings()).ExtractValueSync();
            statement.CheckExecutionStatus(result);
            if (result.IsSuccess()) {
                Tx_.emplace(result.GetTransaction());
            }
            return TStatus(result);
        });
        NStatusHelpers::ThrowOnError(status);
    }
    return *Tx_;
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

SQLRETURN TConnection::Execute(TStatement& statement) {
    // The caller holds the statement operation lock and a shared lifecycle gate.
    std::unique_lock transaction(TransactionMutex_, std::defer_lock);
    const bool explicitTransaction = !GetAutocommit();
    if (explicitTransaction) {
        transaction.lock();
    }
    return statement.ExecuteInternal();
}

void TConnection::EndTranFromEnvironment(SQLSMALLINT completionType) {
    auto check = LockShared();
    if (GetAutocommit()) {
        return;
    }
    check.unlock();
    auto lifecycle = LockExclusive();
    auto operation = LockOperation();
    if (GetAutocommit()) {
        return;
    }
    if (completionType == SQL_COMMIT) {
        CommitTx();
    } else {
        RollbackTx();
    }
}

void TConnection::CloseStatementCursors() {
    // The exclusive lifecycle gate has drained statement/descriptor calls.
    for (auto* child : GetChildren()) {
        if (auto* stmt = dynamic_cast<TStatement*>(child)) {
            stmt->Close(true);
        }
    }
}

void TConnection::InvalidatePreparedStatementMetadata() {
    for (auto* child : GetChildren()) {
        if (auto* stmt = dynamic_cast<TStatement*>(child)) {
            stmt->InvalidatePreparedColumnMeta();
            stmt->InvalidateUnboundParamMetadata();
        }
    }
}

SQLRETURN TConnection::SetAutocommit(bool value) {
    if (value) {
        CommitTx();
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
        InvalidatePreparedStatementMetadata();
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

SQLRETURN TConnection::EndTx(bool commit) {
    if (!Tx_) {
        return SQL_SUCCESS;
    }
    auto status = commit ? TStatus(Tx_->Commit().ExtractValueSync()) : Tx_->Rollback().ExtractValueSync();
    const bool missing = status.GetStatus() == EStatus::NOT_FOUND || status.GetStatus() == EStatus::BAD_SESSION
        || status.GetStatus() == EStatus::SESSION_EXPIRED;
    // Keep uncertain or still-active transactions available for rollback.
    if (status.IsSuccess() || missing || status.GetStatus() == EStatus::ABORTED) {
        Tx_.reset();
        CloseStatementCursors();
    }
    if (missing) {
        if (commit) {
            throw TOdbcException("25S01", static_cast<SQLINTEGER>(status.GetStatus()),
                                "Transaction no longer exists: " + status.GetIssues().ToString());
        }
        return SQL_SUCCESS;
    }
    NStatusHelpers::ThrowOnError(status);
    return SQL_SUCCESS;
}

const std::string& TConnection::GetDataSourceName() const {
    return DataSourceName_;
}

const std::string& TConnection::GetDatabaseName() const {
    return Attributes_.GetCurrentCatalog();
}

const std::string& TConnection::GetServerName() const {
    return ServerName_;
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
                // Version() is unavailable on YDB 24.1 and 24.2. SQL_DBMS_VER is
                // mandatory metadata, so report an unknown version on those servers.
                const auto issues = result.GetIssues().ToOneLineString();
                if (issues.find("Unknown builtin: Version") != std::string::npos) {
                    fetched = "00.00.0000";
                    return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
                }
                return result;
            }
            if (result.GetResultSets().empty()) {
                return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            }
            TResultSetParser parser(result.GetResultSetParser(0));
            if (parser.TryNextRow()) {
                fetched = parser.ColumnParser(0).GetString();
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
    InvalidatePreparedStatementMetadata();
    CloseStatementCursors();
    DestroyYdbState();
    DbmsVersionCache_.reset();
    Ydb_.emplace(*DriverConfig_);
}

void TConnection::ApplyResolvedSettings(TResolvedConnectionSettings&& settings) {
    TConnectionAttributes::NormalizeCatalogPath(settings.Database);
    settings.DriverConfig.SetDatabase(settings.Database);

    Database_ = std::move(settings.Database);
    ServerName_ = std::move(settings.Endpoint);
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
    const std::string nativeSql = RewriteOdbcEscapes(inSql);
    return Diag::WriteOptionalOdbcString(*this, nativeSql, outSql, outMax, outLen);
}

} // namespace NYdb::NOdbc
