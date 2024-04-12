add_library(client-ydb_common_client-impl)

target_link_libraries(client-ydb_common_client-impl PUBLIC
  yutil
  impl-ydb_internal-grpc_connections
)

target_sources(client-ydb_common_client-impl PRIVATE
<<<<<<<< HEAD:src/client/common_client/impl/CMakeLists.txt
<<<<<<< HEAD:src/client/common_client/impl/CMakeLists.txt
  client.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_common_client/impl/client.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_common_client/impl/CMakeLists.txt
========
  client.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_common_client/impl/CMakeLists.txt
)


