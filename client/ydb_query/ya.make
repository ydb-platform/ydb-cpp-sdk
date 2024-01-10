LIBRARY()

SRCS(
    client.cpp
    client.h
    query.cpp
    query.h
    stats.cpp
    stats.h
    tx.cpp
    tx.h
)

PEERDIR(
    client/impl/ydb_internal/make_request
    client/impl/ydb_internal/kqp_session_common
    client/impl/ydb_internal/session_pool
    client/impl/ydb_internal/retry
    client/ydb_common_client
    client/ydb_driver
    client/ydb_query/impl
    client/ydb_result
    client/ydb_types/operation
)

END()
