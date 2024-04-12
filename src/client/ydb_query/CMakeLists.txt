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
<<<<<<<< HEAD:src/client/query/CMakeLists.txt
<<<<<<< HEAD:src/client/query/CMakeLists.txt
========
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_query/CMakeLists.txt
  client.cpp
  query.cpp
  stats.cpp
  tx.cpp
<<<<<<<< HEAD:src/client/query/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_query/client.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_query/query.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_query/stats.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_query/tx.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_query/CMakeLists.txt
========
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_query/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Query ALIAS cpp-client-ydb_query)
