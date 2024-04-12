#define INCLUDE_YDB_INTERNAL_H
#include "status.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/builder.h>
=======
#include <src/util/string/builder.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYdb {

using std::string;

TPlainStatus::TPlainStatus(
    const NYdbGrpc::TGrpcStatus& grpcStatus,
    const string& endpoint,
    std::multimap<std::string, std::string>&& metadata)
    : Endpoint(endpoint)
    , Metadata(std::move(metadata))
{
    std::string msg;
    if (grpcStatus.InternalError) {
        Status = EStatus::CLIENT_INTERNAL_ERROR;
        if (!grpcStatus.Msg.empty()) {
            msg = TStringBuilder() << "Internal client error: " << grpcStatus.Msg;
        } else {
            msg = "Unknown internal client error";
        }
    } else if (grpcStatus.GRpcStatusCode != grpc::StatusCode::OK) {
        switch (grpcStatus.GRpcStatusCode) {
            case grpc::StatusCode::UNAVAILABLE:
                Status = EStatus::TRANSPORT_UNAVAILABLE;
                break;
            case grpc::StatusCode::CANCELLED:
                Status = EStatus::CLIENT_CANCELLED;
                break;
            case grpc::StatusCode::UNAUTHENTICATED:
                Status = EStatus::CLIENT_UNAUTHENTICATED;
                break;
            case grpc::StatusCode::UNIMPLEMENTED:
                Status = EStatus::CLIENT_CALL_UNIMPLEMENTED;
                break;
            case grpc::StatusCode::RESOURCE_EXHAUSTED:
                Status = EStatus::CLIENT_RESOURCE_EXHAUSTED;
                break;
            case grpc::StatusCode::DEADLINE_EXCEEDED:
                Status = EStatus::CLIENT_DEADLINE_EXCEEDED;
                break;
            case grpc::StatusCode::OUT_OF_RANGE:
                Status = EStatus::CLIENT_OUT_OF_RANGE;
                break;
            default:
                Status = EStatus::CLIENT_INTERNAL_ERROR;
                break;
        }
        msg = TStringBuilder() << "GRpc error: (" << grpcStatus.GRpcStatusCode << "): " << grpcStatus.Msg;
    } else {
        Status = EStatus::SUCCESS;
    }
    if (!msg.empty()) {
        Issues.AddIssue(NYql::TIssue(msg));
    }
    for (const auto& [name, value] : grpcStatus.ServerTrailingMetadata) {
        Metadata.emplace(
            std::string(name.begin(), name.end()),
            std::string(value.begin(), value.end())
        );
    }
}

TPlainStatus TPlainStatus::Internal(const std::string& message) {
    return { EStatus::CLIENT_INTERNAL_ERROR, "Internal client error: " + message };
}

} // namespace NYdb
