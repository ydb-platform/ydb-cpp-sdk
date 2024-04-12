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
<<<<<<<< HEAD:src/client/table/impl/CMakeLists.txt
<<<<<<< HEAD:src/client/table/impl/CMakeLists.txt
========
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/CMakeLists.txt
  client_session.cpp
  data_query.cpp
  readers.cpp
  request_migrator.cpp
  table_client.cpp
<<<<<<<< HEAD:src/client/table/impl/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/client_session.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/data_query.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/readers.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/request_migrator.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/impl/table_client.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_table/impl/CMakeLists.txt
========
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/CMakeLists.txt
)


