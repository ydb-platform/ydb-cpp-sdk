get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_import)

target_link_libraries(cpp-client-ydb_import PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
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

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_import PUBLIC
    contrib-libs-linux-headers
  )
endif()
