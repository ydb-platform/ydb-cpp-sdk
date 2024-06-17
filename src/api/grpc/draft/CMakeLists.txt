file(GLOB API_GRPC_DRAFT_SOURCES
  ./*.proto
)

_ydb_sdk_add_grpc_library(api-grpc-draft
  SOURCES ${API_GRPC_DRAFT_SOURCES}
)
_ydb_sdk_install_targets(TARGETS api-grpc-draft)
