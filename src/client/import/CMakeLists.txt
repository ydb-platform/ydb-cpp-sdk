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
  import.cpp
)

generate_enum_serilization(cpp-client-ydb_import
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/import/import.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/import/import.h
)

add_library(YDB-CPP-SDK::Import ALIAS cpp-client-ydb_import)
