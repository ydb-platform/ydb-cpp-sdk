file(GLOB_RECURSE API_CLIENT_YC_PRIVATE_SOURCES
  ./*.proto
)

_ydb_sdk_add_proto_library(api-client-yc_private USE_API_COMMON_PROTOS GRPC
  SOURCES ${API_CLIENT_YC_PRIVATE_SOURCES}
)
_ydb_sdk_install_targets(TARGETS api-client-yc_private)
