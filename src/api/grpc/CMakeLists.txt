add_subdirectory(draft)

file(GLOB API_GRPC_SOURCES
  ./*.proto
)

_ydb_sdk_add_grpc_library(api-grpc
  SOURCES ${API_GRPC_SOURCES}
)
_ydb_sdk_install_targets(TARGETS api-grpc)
