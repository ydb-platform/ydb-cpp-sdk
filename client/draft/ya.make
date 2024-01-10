LIBRARY()

SRCS(
    ydb_dynamic_config.cpp
    ydb_scripting.cpp
    ydb_long_tx.cpp
)

PEERDIR(
    ydb/public/api/grpc/draft
    client/ydb_table
    client/ydb_types/operation
    client/ydb_value
)

END()

RECURSE_FOR_TESTS(
    ut
)
