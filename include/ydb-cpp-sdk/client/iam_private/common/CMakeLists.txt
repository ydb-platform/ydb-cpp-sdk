_ydb_sdk_add_library(client-iam_private-types INTERFACE)

target_link_libraries(client-iam_private-types
  INTERFACE
    client-iam-types
)

_ydb_sdk_install_targets(client-iam_private-types)
