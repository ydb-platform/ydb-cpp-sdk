add_library(cpp-client-ydb_operation)

target_link_libraries(cpp-client-ydb_operation PUBLIC
  yutil
  api-grpc
  public-lib-operation_id
  cpp-client-ydb_query
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_export
  cpp-client-ydb_import
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_operation PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_operation/operation.cpp
)

add_library(YDB-CPP-SDK::Operation ALIAS cpp-client-ydb_operation)
