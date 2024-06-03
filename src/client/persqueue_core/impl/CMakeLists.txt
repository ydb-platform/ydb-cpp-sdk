add_library(client-ydb_persqueue_core-impl INTERFACE)

target_link_libraries(client-ydb_persqueue_core-impl INTERFACE
  client-ydb_topic-impl
)
