LIBRARY()

SRCS(
    session_pool.cpp
)

PEERDIR(
    library/cpp/threading/future
    ydb/public/api/protos
    client/impl/ydb_endpoints
    client/ydb_types/operation
    ydb/library/yql/public/issue/protos
)

END()
