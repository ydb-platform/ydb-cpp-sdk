add_library(client-ydb_topic-common)

target_link_libraries(client-ydb_topic-common PUBLIC
  client-ydb_common_client-impl
  cpp-client-ydb_types
  cpp-monlib-dynamic_counters
  library-cpp-retry
)

target_sources(client-ydb_topic-common PRIVATE
  executor_impl.cpp
  retry_policy.cpp
)
