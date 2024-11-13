_ydb_sdk_add_library(client-iam-common INTERFACE)

target_link_libraries(client-iam-common
  INTERFACE
    client-ydb_types-credentials
    grpc-client
    library-jwt
    threading-future
    yutil
)

_ydb_sdk_install_targets(client-iam-common)
