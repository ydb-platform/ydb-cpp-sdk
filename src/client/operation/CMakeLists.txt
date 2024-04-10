add_library(cpp-client-ydb_operation)

target_link_libraries(cpp-client-ydb_operation PUBLIC
  yutil
  api-grpc
  library-operation_id
  cpp-client-ydb_query
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_export
  cpp-client-ydb_import
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_operation PRIVATE
<<<<<<< HEAD:src/client/operation/CMakeLists.txt
  operation.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_operation/operation.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_operation/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Operation ALIAS cpp-client-ydb_operation)
