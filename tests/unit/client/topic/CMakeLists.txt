add_ydb_test(NAME client-topic-deferred-publication_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    ${YDB_SDK_SOURCE_DIR}/src/client/topic/ut/deferred_publication_ack_state_ut.cpp
    ${YDB_SDK_SOURCE_DIR}/src/client/topic/ut/producer_deferred_publication_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Topic
    client-ydb_topic-impl
  LABELS
    unit
)

add_ydb_test(NAME client-topic-write-session-size_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    ${YDB_SDK_SOURCE_DIR}/src/client/topic/ut/write_session_size_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Topic
    client-ydb_topic-impl
  LABELS
    unit
)

add_ydb_test(NAME client-topic-read-session-credentials_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    ${YDB_SDK_SOURCE_DIR}/src/client/topic/ut/read_session_credentials_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Topic
  LABELS
    unit
)

add_ydb_test(NAME client-topic-read-session-accounting_ut
  INCLUDE_DIRS ${YDB_SDK_SOURCE_DIR}
  SOURCES ${YDB_SDK_SOURCE_DIR}/src/client/topic/ut/read_session_accounting_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Topic
    client-ydb_topic-impl
  LABELS unit
)

add_ydb_test(NAME client-topic-read-session-replication_ut
  INCLUDE_DIRS ${YDB_SDK_SOURCE_DIR}
  SOURCES ${YDB_SDK_SOURCE_DIR}/src/client/topic/ut/replication/read_session_replication_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Topic
    api-grpc
    cpp-testing-common
    containers-disjoint_interval_tree
  LABELS unit
)

add_ydb_test(NAME client-topic-producer-endpoint_ut
  SOURCES ${YDB_SDK_SOURCE_DIR}/src/client/topic/ut_producer_endpoint/endpoint_ut.cpp
  LINK_LIBRARIES YDB-CPP-SDK::Topic
  LABELS unit
)

# producer_retry_policy_ut.cpp, read_session_kafka_timestamps_ut.cpp,
# reset_offset_ut.cpp, and write_session_connect_ut.cpp use the upstream
# TTopicSdkTestSetup and ydb/core server fixtures, which are absent from the
# standalone SDK repository.
