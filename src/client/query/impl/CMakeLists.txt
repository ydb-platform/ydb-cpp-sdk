add_library(client-ydb_query-impl)

target_link_libraries(client-ydb_query-impl PUBLIC
  yutil
  api-grpc-draft
  api-protos
  client-ydb_common_client-impl
  cpp-client-ydb_proto
)

target_sources(client-ydb_query-impl PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/query/impl/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/query/impl/CMakeLists.txt
  exec_query.cpp
  client_session.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_query/impl/exec_query.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_query/impl/client_session.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_query/impl/CMakeLists.txt
========
  exec_query.cpp
  client_session.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_query/impl/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_query/impl/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


