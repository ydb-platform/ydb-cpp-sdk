file(GLOB_RECURSE API_CLIENT_YC_PUBLIC_SOURCES
  ./*.proto
)

_ydb_sdk_add_grpc_library(api-client-yc_public USE_API_COMMON_PROTOS
  SOURCES ${API_CLIENT_YC_PUBLIC_SOURCES}
)
_ydb_sdk_install_targets(TARGETS api-client-yc_public)
