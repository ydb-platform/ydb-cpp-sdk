LIBRARY()

SRCS(
    login.cpp
)

PEERDIR(
    ydb/library/login
    ydb/public/api/grpc
    client/ydb_types/status
    client/impl/ydb_internal/grpc_connections
    ydb/library/yql/public/issue
)

END()
