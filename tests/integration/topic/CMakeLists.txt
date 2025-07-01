add_subdirectory(setup)
add_subdirectory(utils)

add_ydb_test(NAME topic_it GTEST
  SOURCES
    basic_usage.cpp
    describe_topic.cpp
    local_partition.cpp
    topic_to_table.cpp
    trace.cpp
  LINK_LIBRARIES
    topic_it_utils
    topic_it_setup
    cpp-client-ydb_persqueue_public
    api-grpc
    yutil
    YDB-CPP-SDK::Topic
  LABELS
    integration
)

add_ydb_test(NAME topic_direct_read_it GTEST
  SOURCES
    basic_usage.cpp
    describe_topic.cpp
    local_partition.cpp
    direct_read.cpp
    topic_to_table.cpp
  LINK_LIBRARIES
    topic_it_utils
    topic_it_setup
    cpp-client-ydb_persqueue_public
    api-grpc
    yutil
    YDB-CPP-SDK::Topic
  LABELS
    integration
  ENV
    PQ_EXPERIMENTAL_DIRECT_READ=1
)

target_compile_definitions(topic_direct_read_it PUBLIC PQ_EXPERIMENTAL_DIRECT_READ=1)
