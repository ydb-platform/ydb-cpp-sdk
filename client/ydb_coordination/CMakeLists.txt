#add_subdirectory(ut)

get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_coordination)

target_link_libraries(cpp-client-ydb_coordination PUBLIC
  
  yutil
  tools-enum_parser-enum_serialization_runtime
  api-grpc
  impl-ydb_internal-make_request
  cpp-client-ydb_common_client
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_proto
  cpp-client-ydb_types
  client-ydb_types-status
)

target_sources(cpp-client-ydb_coordination PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_coordination/coordination.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_coordination/proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_coordination
  ${CMAKE_SOURCE_DIR}/client/ydb_coordination/coordination.h
  INCLUDE_HEADERS
  client/ydb_coordination/coordination.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_coordination PUBLIC
    
  )
endif()
