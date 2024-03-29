LIBRARY()

SRCS(
    params.cpp
    impl.cpp
)

PEERDIR(
    ydb/public/api/protos
    client/ydb_types/fatal_error_handlers
    client/ydb_value
)

END()

RECURSE_FOR_TESTS(
    ut
)
