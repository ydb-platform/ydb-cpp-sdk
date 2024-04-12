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
  client_session.cpp
  data_query.cpp
  readers.cpp
  request_migrator.cpp
  table_client.cpp
)


