#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/client/types/status_codes.h>
#include <ydb-cpp-sdk/client/types/ydb.h>

#include <ydb-cpp-sdk/library/yql/public/issue/yql_issue.h>

#include <ydb-cpp-sdk/library/grpc/client/grpc_client_low.h>
<<<<<<< HEAD
=======
#include <src/client/ydb_types/status_codes.h>
#include <src/client/ydb_types/ydb.h>

#include <src/library/yql/public/issue/yql_issue.h>

#include <src/library/grpc/client/grpc_client_low.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main


namespace NYdb {

// Other callbacks
using TSimpleCb = std::function<void()>;
using TErrorCb = std::function<void(NYdbGrpc::TGrpcStatus&)>;

struct TBalancingSettings {
    EBalancingPolicy Policy;
    std::string PolicyParams;
};

} // namespace NYdb
