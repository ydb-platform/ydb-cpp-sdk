LIBRARY()

GENERATE_ENUM_SERIALIZATION(client/ydb_topic/topic.h)

SRCS(
    topic.h
    proto_accessor.cpp
)

PEERDIR(
    client/ydb_topic/codecs

    library/cpp/retry
    client/ydb_topic/impl
    client/ydb_proto
    client/ydb_driver
    ydb/public/api/grpc
    ydb/public/api/grpc/draft
    ydb/public/api/protos
    client/ydb_table
)

END()

RECURSE_FOR_TESTS(
    ut
)
