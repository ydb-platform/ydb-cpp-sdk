LIBRARY()

SRCS(
    driver.cpp
)

PEERDIR(
    client/impl/ydb_internal/common
    client/impl/ydb_internal/grpc_connections
    client/resources
    client/ydb_common_client
    client/ydb_types/status
)

END()

RECURSE_FOR_TESTS(
    ut
)
