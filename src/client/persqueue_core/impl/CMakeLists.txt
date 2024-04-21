add_library(client-ydb_persqueue_core-impl)

target_link_libraries(client-ydb_persqueue_core-impl PUBLIC
  yutil
  cpp-containers-disjoint_interval_tree
  library-grpc-client
  cpp-monlib-dynamic_counters
  cpp-monlib-metrics
  cpp-string_utils-url
  library-persqueue-obfuscate
  api-grpc-draft
  impl-ydb_internal-make_request
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-string_utils-misc
)

target_sources(client-ydb_persqueue_core-impl PRIVATE
  read_session_messages.cpp
  common.cpp
  write_session.cpp
  write_session_impl.cpp
  read_session.cpp
  persqueue.cpp
  persqueue_impl.cpp
)
