LIBRARY()

SRCS(
    actions.cpp
    grpc_connections.cpp
)

PEERDIR(
    ydb/public/api/grpc
    ydb/public/api/protos
    client/impl/ydb_internal/db_driver_state
    client/impl/ydb_internal/plain_status
    client/impl/ydb_internal/thread_pool
    client/impl/ydb_stats
    client/resources
    client/ydb_types/exceptions
)

END()
