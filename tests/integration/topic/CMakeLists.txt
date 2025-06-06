add_subdirectory(utils)

add_ydb_test(NAME topic_it GTEST
  SOURCES
    basic_usage.cpp
    describe_topic.cpp
    fixture.cpp
    local_partition.cpp
    trace.cpp
  LINK_LIBRARIES
    topic_it_utils
    cpp-client-ydb_persqueue_public
    YDB-CPP-SDK::Discovery
    api-grpc
    yutil
  LABELS
    integration
)
