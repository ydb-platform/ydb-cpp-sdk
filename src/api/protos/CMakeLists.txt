file(GLOB_RECURSE API_PROTOS_SOURCES
  ./*.proto
)

_ydb_sdk_add_proto_library(api-protos
  SOURCES ${API_PROTOS_SOURCES}
)
_ydb_sdk_install_targets(TARGETS api-protos)
