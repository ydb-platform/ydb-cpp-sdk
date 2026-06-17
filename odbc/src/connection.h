#pragma once

#include "environment.h"
#include "connection_attr.h"
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
#include <vector>

namespace NYdb {
namespace NOdbc {

class TStatement;

class TConnection : public TErrorManager {
private:
    struct TYdbState {
        TDriver Driver;
        NQuery::TQueryClient QueryClient;
        NScheme::TSchemeClient SchemeClient;
        NTable::TTableClient TableClient;

        TYdbState(const std::string& endpoint, const std::string& database)
            : Driver(TDriverConfig().SetEndpoint(endpoint).SetDatabase(database))
            , QueryClient(Driver)
            , SchemeClient(Driver)
            , TableClient(Driver)
        {}
    };

    std::optional<TYdbState> Ydb_;
    std::optional<NQuery::TTransaction> Tx_;
    std::optional<NQuery::TSession> QuerySession_;

    std::vector<std::unique_ptr<TStatement>> Statements_;
    std::string Endpoint_;
    std::string Database_;
    std::string DataSourceName_;
    std::string AuthToken_;
    TEnvironment* ParentEnv_;

    TConnectionAttributes Attributes_;
    mutable std::optional<std::string> DbmsVersionCache_;

    void RecreateYdbClients();
    void RebindToDatabase(const std::string& newDatabase);
public:
    SQLRETURN Connect(const std::string& serverName,
                      const std::string& userName,
                      const std::string& auth);

    SQLRETURN DriverConnect(const std::string& connectionString);
    SQLRETURN Disconnect();

    std::unique_ptr<TStatement> CreateStatement();
    void RemoveStatement(TStatement* stmt);

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
};

} // namespace NOdbc
} // namespace NYdb
