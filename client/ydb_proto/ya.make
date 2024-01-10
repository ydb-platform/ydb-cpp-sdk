LIBRARY()

SRCS(
    accessor.cpp
)

PEERDIR(
    ydb/public/api/grpc
    ydb/public/api/grpc/draft
    ydb/public/api/protos
    ydb/public/lib/operation_id/protos
    client/ydb_params
    client/ydb_value
    ydb/library/yql/public/issue/protos
)

END()
