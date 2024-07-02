file(GLOB_RECURSE LIBRARY_RETRY_PROTOS
  ./*.proto
)

_ydb_sdk_add_proto_library(library-retry-protos
  SOURCES ${LIBRARY_RETRY_PROTOS}
)
_ydb_sdk_install_targets(TARGETS library-retry-protos)
