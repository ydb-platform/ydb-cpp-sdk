add_library(client-ydb_operation)

target_link_libraries(client-ydb_operation PUBLIC
  yutil
  api-grpc
  library-operation_id
  client-ydb_query
  client-ydb_common_client-impl
  client-ydb_driver
  client-ydb_export
  client-ydb_import
  client-ss_tasks
  client-ydb_types-operation
)

target_sources(client-ydb_operation PRIVATE
  operation.cpp
)

_ydb_sdk_install_targets(TARGETS client-ydb_operation)

add_library(YDB-CPP-SDK::Operation ALIAS client-ydb_operation)
