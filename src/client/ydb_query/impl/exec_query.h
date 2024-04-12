#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

<<<<<<<< HEAD:src/client/query/impl/exec_query.h
#include <ydb-cpp-sdk/client/query/client.h>
#include <ydb-cpp-sdk/client/query/tx.h>
#include <src/client/impl/ydb_internal/grpc_connections/grpc_connections.h>
#include <ydb-cpp-sdk/client/params/params.h>
========
#include <src/client/ydb_query/client.h>
#include <src/client/ydb_query/tx.h>
#include <src/client/impl/ydb_internal/grpc_connections/grpc_connections.h>
#include <src/client/ydb_params/params.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_query/impl/exec_query.h

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
