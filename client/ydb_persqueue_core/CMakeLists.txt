add_subdirectory(impl)
# add_subdirectory(ut)

get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_persqueue_core)

target_link_libraries(cpp-client-ydb_persqueue_core PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-retry
  client-ydb_persqueue_core-impl
  cpp-client-ydb_proto
  cpp-client-ydb_driver
  api-grpc
  api-grpc-draft
  api-protos
)

generate_enum_serilization(cpp-client-ydb_persqueue_core
  ${CMAKE_SOURCE_DIR}/client/ydb_persqueue_core/persqueue.h
  INCLUDE_HEADERS
  client/ydb_persqueue_core/persqueue.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux"  AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_persqueue_core PUBLIC
    
  )
endif()
