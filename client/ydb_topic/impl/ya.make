LIBRARY()

SRCS(
    executor.h
    executor.cpp
    read_session_event.cpp
    counters.cpp
    deferred_commit.cpp
    event_handlers.cpp
    read_session.h
    read_session.cpp
    write_session.h
    write_session.cpp
    write_session_impl.h
    write_session_impl.cpp
    topic_impl.h
    topic_impl.cpp
    topic.cpp
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
