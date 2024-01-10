LIBRARY()

GENERATE_ENUM_SERIALIZATION(client/ydb_federated_topic/federated_topic.h)

SRCS(
    federated_topic.h
)

PEERDIR(
    client/ydb_topic
    client/ydb_federated_topic/impl
)

END()

RECURSE_FOR_TESTS(
    ut
)
