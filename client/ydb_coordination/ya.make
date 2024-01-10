LIBRARY()

SRCS(
    coordination.cpp
    proto_accessor.cpp
)

GENERATE_ENUM_SERIALIZATION(coordination.h)

PEERDIR(
    ydb/public/api/grpc
    client/impl/ydb_internal/make_request
    client/ydb_common_client
    client/ydb_common_client/impl
    client/ydb_driver
    client/ydb_proto
    client/ydb_types
    client/ydb_types/status
)

END()

RECURSE_FOR_TESTS(
    ut
)
