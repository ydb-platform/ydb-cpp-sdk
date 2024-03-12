get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_monitoring)

target_link_libraries(cpp-client-ydb_monitoring PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-client-ydb_proto
  impl-ydb_internal-make_request
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_monitoring PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_monitoring/monitoring.cpp
)

generate_enum_serilization(cpp-client-ydb_monitoring
  ${CMAKE_SOURCE_DIR}/client/ydb_monitoring/monitoring.h
  INCLUDE_HEADERS
  client/ydb_monitoring/monitoring.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_monitoring PUBLIC
    
  )
endif()
