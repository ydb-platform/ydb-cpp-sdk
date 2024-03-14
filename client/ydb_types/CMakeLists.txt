add_subdirectory(credentials)
add_subdirectory(exceptions)
add_subdirectory(fatal_error_handlers)
add_subdirectory(operation)
add_subdirectory(status)
get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)
get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_types)

target_link_libraries(cpp-client-ydb_types PUBLIC
  yutil
  protobuf::libprotobuf
  library-grpc-client
  yql-public-issue
  tools-enum_parser-enum_serialization_runtime
)
generate_enum_serilization(cpp-client-ydb_types
  ${CMAKE_SOURCE_DIR}/client/ydb_types/s3_settings.h
  INCLUDE_HEADERS
  client/ydb_types/s3_settings.h
)
generate_enum_serilization(cpp-client-ydb_types
  ${CMAKE_SOURCE_DIR}/client/ydb_types/status_codes.h
  INCLUDE_HEADERS
  client/ydb_types/status_codes.h
)


