#pragma once

#include "environment.h"
#include "utils/error_manager.h"

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/query/client.h>
#include <ydb-cpp-sdk/client/scheme/scheme.h>
#include <ydb-cpp-sdk/client/table/table.h>

#include <sql.h>
#include <sqlext.h>

#include <memory>
#include <vector>
#include <string>

namespace NYdb {
namespace NOdbc {

class TStatement;

class TConnection : public TErrorManager {
private:
    std::unique_ptr<TDriver> YdbDriver_;
    std::unique_ptr<NQuery::TQueryClient> YdbClient_;
    std::unique_ptr<NTable::TTableClient> YdbTableClient_;
    std::unique_ptr<NScheme::TSchemeClient> YdbSchemeClient_;
    std::optional<NQuery::TTransaction> Tx_;

    std::vector<std::unique_ptr<TStatement>> Statements_;
    std::string Endpoint_;
    std::string Database_;
    std::string AuthToken_;

    bool Autocommit_ = true;

public:
    SQLRETURN Connect(const std::string& serverName,
                      const std::string& userName,
                      const std::string& auth);

    SQLRETURN DriverConnect(const std::string& connectionString);
    SQLRETURN Disconnect();

    std::unique_ptr<TStatement> CreateStatement();
    void RemoveStatement(TStatement* stmt);

    NYdb::NQuery::TQueryClient* GetClient() { return YdbClient_.get(); }
    NYdb::NTable::TTableClient* GetTableClient() { return YdbTableClient_.get(); }
    NScheme::TSchemeClient* GetSchemeClient() { return YdbSchemeClient_.get(); }

    SQLRETURN SetAutocommit(bool value);
    bool GetAutocommit() const;

    const std::optional<NQuery::TTransaction>& GetTx();
    void SetTx(const NQuery::TTransaction& tx);

    SQLRETURN CommitTx();
    SQLRETURN RollbackTx();
};

} // namespace NOdbc
} // namespace NYdb
