add_library(client-ydb_federated_topic-impl)

target_link_libraries(client-ydb_federated_topic-impl PUBLIC
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
)

target_sources(client-ydb_federated_topic-impl PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_federated_topic/impl/federated_read_session.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_federated_topic/impl/federated_read_session_event.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_federated_topic/impl/federated_write_session.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_federated_topic/impl/federated_topic_impl.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_federated_topic/impl/federated_topic.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_federated_topic/impl/federation_observer.cpp
)


