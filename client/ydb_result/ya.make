LIBRARY()

SRCS(
    proto_accessor.cpp
    result.cpp
)

PEERDIR(
    ydb/public/api/protos
    client/ydb_types/fatal_error_handlers
    client/ydb_value
    client/ydb_proto
)

END()

RECURSE_FOR_TESTS(
    ut
)
