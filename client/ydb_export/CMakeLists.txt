add_library(cpp-client-ydb_export)

target_link_libraries(cpp-client-ydb_export PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  api-grpc
  api-protos
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_proto
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_export PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_export/export.cpp
)

generate_enum_serilization(cpp-client-ydb_export
  ${CMAKE_SOURCE_DIR}/client/ydb_export/export.h
  INCLUDE_HEADERS
  client/ydb_export/export.h
)

add_library(YDB-CPP-SDK::Export ALIAS cpp-client-ydb_export)
