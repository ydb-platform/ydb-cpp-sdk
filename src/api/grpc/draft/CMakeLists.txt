file(GLOB API_GRPC_DRAFT_SOURCES
  *.proto
)

_ydb_sdk_add_proto_library(api-grpc-draft GRPC
  SOURCES ${API_GRPC_DRAFT_SOURCES}
)
target_link_libraries(api-grpc-draft PUBLIC
  api-protos
)
_ydb_sdk_install_targets(TARGETS api-grpc-draft)
