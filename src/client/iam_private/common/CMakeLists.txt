_ydb_sdk_add_library(client-iam_private-common INTERFACE)

target_link_libraries(client-iam_private-common
  INTERFACE
    client-iam-types
    client-iam_private-types
)

_ydb_sdk_install_targets(TARGETS client-iam_private-common)
