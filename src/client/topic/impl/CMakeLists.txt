add_library(client-ydb_topic-impl)

target_link_libraries(client-ydb_topic-impl PUBLIC
  yutil
  library-grpc-client
  cpp-monlib-dynamic_counters
  cpp-monlib-metrics
  cpp-string_utils-url
  library-persqueue-obfuscate
  api-grpc-draft
  api-grpc
  impl-ydb_internal-make_request
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  client-ydb_persqueue_core-impl
  cpp-client-ydb_proto
  proto_output
)

target_sources(client-ydb_topic-impl PRIVATE
  executor.cpp
  read_session_event.cpp
  counters.cpp
  deferred_commit.cpp
  event_handlers.cpp
  read_session.cpp
  write_session.cpp
  write_session_impl.cpp
  topic_impl.cpp
  topic.cpp
)


