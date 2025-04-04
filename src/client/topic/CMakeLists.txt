add_subdirectory(common)
add_subdirectory(impl)
add_subdirectory(codecs)

_ydb_sdk_add_library(client-ydb_topic)

target_link_libraries(client-ydb_topic PUBLIC
  yutil
  enum_serialization_runtime
  client-ydb_topic-codecs
  retry
  client-ydb_topic-common
  client-ydb_topic-impl
  client-ydb_topic-include
  client-ydb_proto
  client-ydb_driver
  api-grpc
  api-grpc-draft
  api-protos
  client-ydb_table
)

target_sources(client-ydb_topic PRIVATE
  out.cpp
)

_ydb_sdk_make_client_component(Topic client-ydb_topic)
