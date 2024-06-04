add_library(client-ydb_persqueue_public-impl)

target_link_libraries(client-ydb_persqueue_public-impl PUBLIC
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

target_sources(client-ydb_persqueue_public-impl PRIVATE
  common.cpp
  persqueue_impl.cpp
  persqueue.cpp
  read_session.cpp
  read_session_messages.cpp
  write_session_impl.cpp
  write_session.cpp
)
