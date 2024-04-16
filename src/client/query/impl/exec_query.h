#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

#include <src/client/query/client.h>
#include <src/client/query/tx.h>
#include <src/client/impl/ydb_internal/grpc_connections/grpc_connections.h>
#include <src/client/params/params.h>

namespace NYdb::NQuery {

class TExecQueryImpl {
public:
    static TAsyncExecuteQueryIterator StreamExecuteQuery(const std::shared_ptr<TGRpcConnectionsImpl>& connections,
        const TDbDriverStatePtr& driverState, const std::string& query, const TTxControl& txControl,
        const std::optional<TParams>& params, const TExecuteQuerySettings& settings, const std::optional<TSession>& session);

    static TAsyncExecuteQueryResult ExecuteQuery(const std::shared_ptr<TGRpcConnectionsImpl>& connections,
        const TDbDriverStatePtr& driverState, const std::string& query, const TTxControl& txControl,
        const std::optional<TParams>& params, const TExecuteQuerySettings& settings, const std::optional<TSession>& session);
};

} // namespace NYdb::NQuery::NImpl
