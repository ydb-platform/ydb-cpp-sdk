add_library(cpp-client-ydb_value)

target_link_libraries(cpp-client-ydb_value PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-containers-stack_vector
  api-protos
  impl-ydb_internal-value_helpers
  client-ydb_types-fatal_error_handlers
  yql-public-decimal
  ydb-library-uuid
  proto_output
)

target_sources(cpp-client-ydb_value PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value/value.cpp
)

generate_enum_serilization(cpp-client-ydb_value
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value/value.h
  INCLUDE_HEADERS
  client/ydb_value/value.h
)

add_library(YDB-CPP-SDK::Value ALIAS cpp-client-ydb_value)
