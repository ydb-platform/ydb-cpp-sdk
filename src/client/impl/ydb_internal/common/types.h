#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

#include <src/client/types/status_codes.h>
#include <src/client/types/ydb.h>

#include <src/library/yql/public/issue/yql_issue.h>

#include <src/library/grpc/client/grpc_client_low.h>


namespace NYdb {

// Other callbacks
using TSimpleCb = std::function<void()>;
using TErrorCb = std::function<void(NYdbGrpc::TGrpcStatus&)>;

struct TBalancingSettings {
    EBalancingPolicy Policy;
    std::string PolicyParams;
};

} // namespace NYdb
