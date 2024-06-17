add_subdirectory(common)
add_subdirectory(impl)

add_library(client-ydb_topic-codecs INTERFACE)

target_link_libraries(client-ydb_topic-codecs INTERFACE
  yutil
  streams-zstd
  yql-public-issue-protos
  api-grpc-draft
  api-grpc
  api-protos
)

_ydb_sdk_install_targets(TARGETS client-ydb_topic-codecs)

add_library(client-ydb_topic)

target_link_libraries(client-ydb_topic PUBLIC
  yutil
  enum_serialization_runtime
  client-ydb_topic-codecs
  library-retry
  client-ydb_topic-common
  client-ydb_topic-impl
  client-ydb_proto
  client-ydb_driver
  api-grpc
  api-grpc-draft
  api-protos
  client-ydb_table
)

target_sources(client-ydb_topic PRIVATE
  proto_accessor.cpp
  out.cpp
)

generate_enum_serilization(client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/codecs.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/codecs.h
)

generate_enum_serilization(client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/control_plane.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/control_plane.h
)

generate_enum_serilization(client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/read_events.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/read_events.h
)

generate_enum_serilization(client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/write_events.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/write_events.h
)

_ydb_sdk_install_targets(TARGETS client-ydb_topic)

add_library(YDB-CPP-SDK::Topic ALIAS client-ydb_topic)
