#pragma once

#include <client/impl/ydb_internal/internal_header.h>

#include <client/ydb_types/status_codes.h>
#include <client/ydb_types/ydb.h>

#include <ydb/library/yql/public/issue/yql_issue.h>

#include <ydb/library/grpc/client/grpc_client_low.h>


namespace NYdb {

// Other callbacks
using TSimpleCb = std::function<void()>;
using TErrorCb = std::function<void(NYdbGrpc::TGrpcStatus&)>;

struct TBalancingSettings {
    EBalancingPolicy Policy;
    std::string PolicyParams;
};

} // namespace NYdb
