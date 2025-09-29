_ydb_sdk_add_library(impl-internal-grpc_connections)

target_link_libraries(impl-internal-grpc_connections PUBLIC
  yutil
  api-grpc
  api-protos
  impl-internal-db_driver_state
  impl-internal-plain_status
  client-impl-ydb_stats
  client-resources
  client-types-exceptions
  client-types-executor
)

target_sources(impl-internal-grpc_connections PRIVATE
  actions.cpp
  grpc_connections.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-grpc_connections)
