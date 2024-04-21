add_library(cpp-client-ydb_result)

target_link_libraries(cpp-client-ydb_result PUBLIC
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
  cpp-client-ydb_value
  cpp-client-ydb_proto
)

target_sources(cpp-client-ydb_result PRIVATE
  proto_accessor.cpp
  result.cpp
)

add_library(YDB-CPP-SDK::Result ALIAS cpp-client-ydb_result)
