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
  export.cpp
  out.cpp
)

generate_enum_serilization(cpp-client-ydb_export
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/export/export.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/export/export.h
)

add_library(YDB-CPP-SDK::Export ALIAS cpp-client-ydb_export)
