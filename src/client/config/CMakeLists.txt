_ydb_sdk_add_library(client-config)

target_link_libraries(client-config
  PUBLIC
    client-ydb_driver
    client-ydb_common_client
    client-ydb_types-status
    client-ydb_types
  PRIVATE
    api-grpc
    impl-ydb_internal-make_request
    client-ydb_common_client-impl
)

target_sources(client-config
  PRIVATE
    config.cpp
)

_ydb_sdk_make_client_component(Config client-config)
