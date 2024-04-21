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
  client.cpp
  query.cpp
  stats.cpp
  tx.cpp
)

add_library(YDB-CPP-SDK::Query ALIAS cpp-client-ydb_query)
