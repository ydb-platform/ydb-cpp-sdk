LIBRARY()

SRCS(
    db_pool.cpp
)

PEERDIR(
    ydb/library/actors/core
    library/cpp/monlib/dynamic_counters
    ydb/core/protos
    ydb/library/db_pool/protos
    ydb/library/security
    client/ydb_driver
    client/ydb_table
)

END()

RECURSE(
    protos
)
