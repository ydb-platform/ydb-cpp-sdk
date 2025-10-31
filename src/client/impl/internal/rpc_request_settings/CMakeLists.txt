_ydb_sdk_add_library(impl-internal-rpc_request_settings INTERFACE)

target_link_libraries(impl-internal-rpc_request_settings INTERFACE
  yutil
  ydb-time
  client-impl-ydb_endpoints
)

_ydb_sdk_install_targets(TARGETS impl-internal-rpc_request_settings)
