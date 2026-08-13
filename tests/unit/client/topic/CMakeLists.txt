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
