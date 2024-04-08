add_library(client-ydb_table-impl)

target_link_libraries(client-ydb_table-impl PUBLIC
  yutil
  cpp-threading-future
  api-protos
  lib-operation_id-protos
  client-impl-ydb_endpoints
  impl-ydb_internal-session_pool
  client-ydb_table-query_stats
  public-issue-protos
)

target_sources(client-ydb_table-impl PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/client_session.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/data_query.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/readers.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/request_migrator.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/table_client.cpp
)


