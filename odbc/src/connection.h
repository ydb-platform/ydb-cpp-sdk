#pragma once

#include "connection_attr.h"
#include "connection_config.h"
#include "utils/handle.h"

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/query/client.h>
#include <ydb-cpp-sdk/client/scheme/scheme.h>
#include <ydb-cpp-sdk/client/table/table.h>

#include "odbc_compat.h"

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace NYdb::NOdbc {

class TStatement;

class TConnection : public THandle {
private:
    std::shared_mutex Lifecycle_; // Shared by calls; exclusive for connection-state changes and child invalidation.
    std::mutex TransactionMutex_; // Tx_, QuerySession_, TransactionFailed_; also protected by exclusive Lifecycle_.
    std::atomic<uint64_t> CursorGeneration_ = 0;
    bool TransactionFailed_ = false;
    struct TYdbState {
        // Declared first: constructed before clients, destroyed after them.
        TDriver Driver;
        NQuery::TQueryClient QueryClient;
        NScheme::TSchemeClient SchemeClient;
        NTable::TTableClient TableClient;
        std::mutex QuarantineMutex_; // CanceledSessions_ only; lifecycle changes drain callers.
        std::vector<NQuery::TSession> CanceledSessions_;

        explicit TYdbState(const TDriverConfig& config)
            : Driver(config)
            , QueryClient(Driver)
            , SchemeClient(Driver)
            , TableClient(Driver)
        {
            const auto limit = QueryClient.GetActiveSessionsLimit();
            if (limit <= 0) {
                throw TOdbcException("HY000", 0, "Query session pool must be bounded");
            }
            CanceledSessions_.reserve(static_cast<size_t>(limit));
        }

        void QuarantineSession(NQuery::TSession session) {
            std::lock_guard lock(QuarantineMutex_);
            // Each lease is retained once and still counts against the pool limit.
            assert(CanceledSessions_.size() < CanceledSessions_.capacity());
            CanceledSessions_.push_back(std::move(session));
        }

        ~TYdbState() {
            // Stop before retained leases can return to the pool.
            Driver.Stop(true);
        }
    };

    std::optional<TYdbState> Ydb_;
    std::optional<TDriverConfig> DriverConfig_;
    std::optional<NQuery::TTransaction> Tx_;
    std::optional<NQuery::TSession> QuerySession_;

    std::string Database_;
    std::string ServerName_;
    std::string DataSourceName_;

    TConnectionAttributes Attributes_;
    mutable std::optional<std::string> DbmsVersionCache_;

    void DestroyYdbState();
    void ApplyResolvedSettings(TResolvedConnectionSettings&& settings);
    void RecreateYdbClients();
    void RebindToDatabase(std::string_view newDatabase);
    void InvalidatePreparedStatementMetadata();
public:
    TConnection() { SetLifecycle(&Lifecycle_); }

    SQLRETURN Connect(std::string_view serverName,
                      std::string_view userName,
                      std::string_view auth);

    SQLRETURN DriverConnect(std::string_view connectionString, SQLCHAR* outConnectionString,
                            SQLSMALLINT bufferLength, SQLSMALLINT* stringLength2Ptr);
    SQLRETURN Disconnect();

    void CloseStatementCursors();
    uint64_t GetCursorGeneration() const { return CursorGeneration_.load(std::memory_order_relaxed); }
    // Test seam for holding transaction ordering without a server-side operation.
    std::unique_lock<std::mutex> LockTransaction() { return std::unique_lock(TransactionMutex_); }
    SQLRETURN Execute(TStatement& statement);
    void EndTranFromEnvironment(SQLSMALLINT completionType);
    void CheckTransaction() const;
    void FailTransaction();
    void QuarantineSession(NQuery::TSession session);

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
    const std::string& GetDatabaseName() const;
    const std::string& GetServerName() const;
    const std::string& GetDataSourceName() const;
    SQLUINTEGER GetSupportedTxnIsolationOptions() const;
    bool IsDataSourceReadOnly() const;

    const std::optional<NQuery::TTransaction>& GetTx();
    void SetTx(const NQuery::TTransaction& tx);

    SQLRETURN CommitTx();
    SQLRETURN RollbackTx();

    SQLRETURN NativeSql(const std::string& inSql, SQLCHAR* outSql, SQLINTEGER outMax, SQLINTEGER* outLen);
};

} // namespace NYdb::NOdbc
