file(GLOB_RECURSE LIB_OPERATION_ID_PROTOS
  ./*.proto
)

_ydb_sdk_add_proto_library(lib-operation_id-protos
  SOURCES ${LIB_OPERATION_ID_PROTOS}
)
_ydb_sdk_install_targets(TARGETS lib-operation_id-protos)
