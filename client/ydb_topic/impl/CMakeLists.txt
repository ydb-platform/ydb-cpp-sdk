add_library(client-ydb_topic-impl)

target_link_libraries(client-ydb_topic-impl PUBLIC
  contrib-libs-cxxsupp
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
  cpp-string_builder
)

target_sources(client-ydb_topic-impl PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/executor.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/read_session_event.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/counters.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/deferred_commit.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/event_handlers.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/read_session.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/write_session.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/write_session_impl.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/topic_impl.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/impl/topic.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_topic-impl PUBLIC
    contrib-libs-linux-headers
  )
endif()
