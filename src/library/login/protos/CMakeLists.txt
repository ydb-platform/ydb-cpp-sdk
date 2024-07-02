file(GLOB_RECURSE LIBRARY_LOGIN_PROTOS_SOURCES
  ./*.proto
)

_ydb_sdk_add_proto_library(library-login-protos
  SOURCES ${LIBRARY_LOGIN_PROTOS_SOURCES}
)
_ydb_sdk_install_targets(TARGETS library-login-protos)
