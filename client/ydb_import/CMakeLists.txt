add_library(cpp-client-ydb_import)

target_link_libraries(cpp-client-ydb_import PUBLIC
  
  yutil
  tools-enum_parser-enum_serialization_runtime
  api-grpc
  api-protos
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_proto
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_import PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_import/import.cpp
)

generate_enum_serilization(cpp-client-ydb_import
  ${CMAKE_SOURCE_DIR}/client/ydb_import/import.h
  INCLUDE_HEADERS
  client/ydb_import/import.h
)


