# add_subdirectory(ut)

get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_value)

target_link_libraries(cpp-client-ydb_value PUBLIC
  contrib-libs-cxxsupp
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-containers-stack_vector
  api-protos
  impl-ydb_internal-value_helpers
  client-ydb_types-fatal_error_handlers
  yql-public-decimal
  ydb-library-uuid
  cpp-string_builder
)

target_sources(cpp-client-ydb_value PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_value/value.cpp
)

generate_enum_serilization(cpp-client-ydb_value
  ${CMAKE_SOURCE_DIR}/client/ydb_value/value.h
  INCLUDE_HEADERS
  client/ydb_value/value.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_value PUBLIC
    contrib-libs-linux-headers
  )
endif()
