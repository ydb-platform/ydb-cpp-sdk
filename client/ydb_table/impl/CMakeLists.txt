add_library(client-ydb_table-impl)

target_link_libraries(client-ydb_table-impl PUBLIC
  contrib-libs-cxxsupp
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
  ${CMAKE_SOURCE_DIR}/client/ydb_table/impl/client_session.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_table/impl/data_query.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_table/impl/readers.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_table/impl/request_migrator.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_table/impl/table_client.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_table-impl PUBLIC
    contrib-libs-linux-headers
  )
endif()
