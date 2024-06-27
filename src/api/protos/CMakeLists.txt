file(GLOB_RECURSE API_PROTOS_SOURCES
  ./*.proto
)

_ydb_sdk_add_proto_library(api-protos
  SOURCES ${API_PROTOS_SOURCES}
)
target_link_libraries(api-protos PUBLIC
  enum_serialization_runtime
)
_ydb_sdk_install_targets(TARGETS api-protos)
