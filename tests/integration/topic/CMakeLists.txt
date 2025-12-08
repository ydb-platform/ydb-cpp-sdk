add_subdirectory(setup)
add_subdirectory(utils)

add_ydb_test(NAME topic_it GTEST
  SOURCES
    basic_usage_it.cpp
    describe_topic_it.cpp
    local_partition_it.cpp
    topic_to_table_it.cpp
    trace_it.cpp
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
    basic_usage_it.cpp
    describe_topic_it.cpp
    local_partition_it.cpp
    direct_read_it.cpp
    topic_to_table_it.cpp
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
