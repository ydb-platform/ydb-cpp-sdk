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
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_core/impl/read_session_messages.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_core/impl/common.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_core/impl/write_session.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_core/impl/write_session_impl.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_core/impl/read_session.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_core/impl/persqueue.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_core/impl/persqueue_impl.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_persqueue_core-impl PUBLIC
    
  )
endif()
