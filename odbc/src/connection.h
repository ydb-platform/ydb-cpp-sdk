#pragma once

#include "environment.h"
#include "connection_attr.h"
#include "connection_config.h"
#include "utils/error_manager.h"

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/query/client.h>
#include <ydb-cpp-sdk/client/scheme/scheme.h>
#include <ydb-cpp-sdk/client/table/table.h>

#include <sql.h>
#include <sqlext.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

namespace NYdb::NOdbc {

class TStatement;
class TDescriptor;

class TConnection : public TErrorManager {
private:
    struct TYdbState {
        // Declared first: constructed before clients, destroyed after them.
        TDriver Driver;
        NQuery::TQueryClient QueryClient;
        NScheme::TSchemeClient SchemeClient;
        NTable::TTableClient TableClient;

        explicit TYdbState(const TDriverConfig& config)
            : Driver(config)
            , QueryClient(Driver)
            , SchemeClient(Driver)
            , TableClient(Driver)
        {}

        ~TYdbState() {
            Driver.Stop(true);
        }
    };

    std::optional<TYdbState> Ydb_;
    std::optional<TDriverConfig> DriverConfig_;
    std::optional<NQuery::TTransaction> Tx_;
    std::optional<NQuery::TSession> QuerySession_;

    std::string Endpoint_;
    std::string Database_;
    std::string DataSourceName_;
    TEnvironment* ParentEnv_ = nullptr;

    TConnectionAttributes Attributes_;
    mutable std::optional<std::string> DbmsVersionCache_;
    std::unordered_set<TStatement*> Statements_;
    std::unordered_set<TDescriptor*> Descriptors_;

    void DestroyYdbState();
    void ApplyResolvedSettings(TResolvedConnectionSettings&& settings);
    void RecreateYdbClients();
    void RebindToDatabase(std::string_view newDatabase);
public:
    ~TConnection();

    SQLRETURN Connect(std::string_view serverName,
                      std::string_view userName,
                      std::string_view auth);

    SQLRETURN DriverConnect(std::string_view connectionString);
    SQLRETURN Disconnect();

    std::unique_ptr<TStatement> CreateStatement();
    void RegisterStatement(TStatement* stmt) { Statements_.insert(stmt); }
    void UnregisterStatement(TStatement* stmt) { Statements_.erase(stmt); }
    void RegisterDescriptor(TDescriptor* desc) { Descriptors_.insert(desc); }
    void UnregisterDescriptor(TDescriptor* desc) { Descriptors_.erase(desc); }
    bool HasChildren() const noexcept { return !Statements_.empty() || !Descriptors_.empty(); }
    void CloseStatementCursors();

    std::optional<NQuery::TQueryClient> GetClient();
    NQuery::TSession& GetOrCreateQuerySession();
    std::optional<NTable::TTableClient> GetTableClient();
    std::optional<NScheme::TSchemeClient> GetSchemeClient();

    SQLRETURN SetAutocommit(bool value);
    bool GetAutocommit() const;

    SQLRETURN SetConnectAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetConnectAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);
    NQuery::TTxSettings MakeTxSettings() const;

    std::string WrapQueryForCurrentCatalog(const std::string& sql) const;
    TConnectionAttributes::TCatalogBinding GetCatalogBinding() const;
    const std::string& GetDbmsVersion();
    const std::string& GetDataSourceName() const;
    SQLUINTEGER GetSupportedTxnIsolationOptions() const;
    bool IsDataSourceReadOnly() const;

    const std::optional<NQuery::TTransaction>& GetTx();
    void SetTx(const NQuery::TTransaction& tx);
    void ResetTx();
    void ResetQuerySession();

    SQLRETURN CommitTx();
    SQLRETURN RollbackTx();

    void SetEnvironment(TEnvironment* env);
    TEnvironment* GetEnvironment();

    SQLRETURN NativeSql(const std::string& inSql, SQLCHAR* outSql, SQLINTEGER outMax, SQLINTEGER* outLen);
};

} // namespace NYdb::NOdbc
