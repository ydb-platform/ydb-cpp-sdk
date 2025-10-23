_ydb_sdk_add_library(impl-internal-db_driver_state)

target_link_libraries(impl-internal-db_driver_state PUBLIC
  yutil
  string_utils-quote
  threading-future
  client-impl-ydb_endpoints
  impl-internal-logger
  impl-internal-plain_status
  impl-internal-local_dc_detector
  client-types-credentials
)

target_sources(impl-internal-db_driver_state PRIVATE
  authenticator.cpp
  endpoint_pool.cpp
  state.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-db_driver_state)
