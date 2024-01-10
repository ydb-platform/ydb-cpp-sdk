LIBRARY()

SRCS(
    federated_read_session.h
    federated_read_session.cpp
    federated_read_session_event.cpp
    federated_write_session.h
    federated_write_session.cpp
    federated_topic_impl.h
    federated_topic_impl.cpp
    federated_topic.cpp
    federation_observer.h
    federation_observer.cpp
)

PEERDIR(
    ydb/library/grpc/client
    library/cpp/monlib/dynamic_counters
    library/cpp/monlib/metrics
    library/cpp/string_utils/url
    ydb/library/persqueue/obfuscate
    ydb/public/api/grpc/draft
    ydb/public/api/grpc
    client/impl/ydb_internal/make_request
    client/ydb_common_client/impl
    client/ydb_driver
    client/ydb_persqueue_core/impl
    client/ydb_proto
)

END()
