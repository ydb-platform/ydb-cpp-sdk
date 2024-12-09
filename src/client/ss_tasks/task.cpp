#include "task.h"
#include <src/api/grpc/ydb_discovery_v1.grpc.pb.h>
#include <src/api/grpc/ydb_export_v1.grpc.pb.h>
#include <src/api/protos/ydb_export.pb.h>
#include <src/client/common_client/impl/client.h>
#include <ydb-cpp-sdk/client/proto/accessor.h>

namespace NYdb::inline V3 {
namespace NSchemeShard {

/// YT
TBackgroundProcessesResponse::TBackgroundProcessesResponse(TStatus&& status, Ydb::Operations::Operation&& operation)
    : TOperation(std::move(status), std::move(operation))
{
    Metadata_.Id = GetProto().DebugString();
}

const TBackgroundProcessesResponse::TMetadata& TBackgroundProcessesResponse::Metadata() const {
    return Metadata_;
}

} // namespace NExport
} // namespace NYdb
