file(GLOB_RECURSE YQL_PUBLIC_ISSUE_PROTOS
  ./*.proto
)

_ydb_sdk_add_proto_library(yql-public-issue-protos
  SOURCES ${YQL_PUBLIC_ISSUE_PROTOS}
)

_ydb_sdk_install_targets(TARGETS yql-public-issue-protos)
