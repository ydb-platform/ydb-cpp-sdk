add_library(client-ydb_value)

target_link_libraries(client-ydb_value PUBLIC
  yutil
  enum_serialization_runtime
  containers-stack_vector
  api-protos
  impl-ydb_internal-value_helpers
  client-ydb_types-fatal_error_handlers
  yql-public-decimal
  library-uuid
  proto_output
)

target_sources(client-ydb_value PRIVATE
  value.cpp
  out.cpp
)

generate_enum_serilization(client-ydb_value
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/value/value.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/value/value.h
)

_ydb_sdk_install_targets(TARGETS client-ydb_value)

add_library(YDB-CPP-SDK::Value ALIAS client-ydb_value)
