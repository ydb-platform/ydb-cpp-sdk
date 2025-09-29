_ydb_sdk_add_library(client-types-credentials-login)

target_link_libraries(client-types-credentials-login
  PUBLIC
    yutil
  PRIVATE
    api-grpc
    client-types-status
    impl-internal-grpc_connections
    yql-public-issue
    jwt-cpp::jwt-cpp
)

target_sources(client-types-credentials-login PRIVATE
  login.cpp
)

_ydb_sdk_install_targets(TARGETS client-types-credentials-login)
