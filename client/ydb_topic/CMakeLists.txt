add_subdirectory(codecs)
add_subdirectory(impl)

get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-client-ydb_topic)

target_link_libraries(cpp-client-ydb_topic PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  client-ydb_topic-codecs
  library-cpp-retry
  client-ydb_topic-impl
  cpp-client-ydb_proto
  cpp-client-ydb_driver
  api-grpc
  api-grpc-draft
  api-protos
  cpp-client-ydb_table
)

target_sources(cpp-client-ydb_topic PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_topic
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/topic.h
  INCLUDE_HEADERS
  client/ydb_topic/topic.h
)


