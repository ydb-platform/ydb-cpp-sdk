add_subdirectory(common)
add_subdirectory(impl)

add_library(client-ydb_topic-codecs INTERFACE)

target_link_libraries(client-ydb_topic-codecs INTERFACE
  yutil
  cpp-streams-zstd
  public-issue-protos
  api-grpc-draft
  api-grpc
  api-protos
)

add_library(cpp-client-ydb_topic)

target_link_libraries(cpp-client-ydb_topic PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  client-ydb_topic-codecs
  library-cpp-retry
  client-ydb_topic-common
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
  out.cpp
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/codecs.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/codecs.h
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/control_plane.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/control_plane.h
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/read_events.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/read_events.h
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/write_events.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/write_events.h
)

add_library(YDB-CPP-SDK::Topic ALIAS cpp-client-ydb_topic)
