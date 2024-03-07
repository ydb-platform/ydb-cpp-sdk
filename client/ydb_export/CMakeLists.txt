get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

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

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_export PUBLIC
    
  )
endif()
