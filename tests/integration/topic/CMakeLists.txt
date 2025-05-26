add_ydb_test(NAME topic_it GTEST
  SOURCES
    basic_usage.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Topic
    cpp-client-ydb_persqueue_public
    api-grpc
  LABELS
    integration
)
