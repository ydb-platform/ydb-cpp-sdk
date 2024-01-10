LIBRARY()

SRCS(
    operation.cpp
)

PEERDIR(
    ydb/public/api/grpc
    ydb/public/lib/operation_id
    client/ydb_query
    client/ydb_common_client/impl
    client/ydb_driver
    client/ydb_export
    client/ydb_import
    client/ydb_types/operation
)

END()
