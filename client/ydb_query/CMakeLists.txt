add_subdirectory(impl)

add_library(cpp-client-ydb_query)

target_link_libraries(cpp-client-ydb_query PUBLIC
  yutil
  impl-ydb_internal-make_request
  impl-ydb_internal-kqp_session_common
  impl-ydb_internal-session_pool
  impl-ydb_internal-retry
  cpp-client-ydb_common_client
  cpp-client-ydb_driver
  client-ydb_query-impl
  cpp-client-ydb_result
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_query PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_query/client.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_query/query.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_query/stats.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_query/tx.cpp
)

add_library(YDB-CPP-SDK::Query ALIAS cpp-client-ydb_query)
