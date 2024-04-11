add_subdirectory(codecs)
add_subdirectory(impl)

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
  proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_topic/topic.h
  INCLUDE_HEADERS
  src/client/ydb_topic/topic.h
)

add_library(YDB-CPP-SDK::Topic ALIAS cpp-client-ydb_topic)
