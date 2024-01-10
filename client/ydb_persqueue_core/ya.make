LIBRARY()

GENERATE_ENUM_SERIALIZATION(client/ydb_persqueue_core/persqueue.h)

SRCS(
    persqueue.h
)

PEERDIR(
    library/cpp/retry
    client/ydb_persqueue_core/impl
    client/ydb_proto
    client/ydb_driver
    ydb/public/api/grpc
    ydb/public/api/grpc/draft
    ydb/public/api/protos
)

END()

RECURSE_FOR_TESTS(
    ut
)
