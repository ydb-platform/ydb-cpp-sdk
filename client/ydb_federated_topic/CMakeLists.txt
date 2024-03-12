add_subdirectory(impl)

get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_federated_topic)

target_link_libraries(cpp-client-ydb_federated_topic PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-client-ydb_topic
  client-ydb_federated_topic-impl
)

generate_enum_serilization(cpp-client-ydb_federated_topic
  ${CMAKE_SOURCE_DIR}/client/ydb_federated_topic/federated_topic.h
  INCLUDE_HEADERS
  client/ydb_federated_topic/federated_topic.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_federated_topic PUBLIC
   
  )
endif()
