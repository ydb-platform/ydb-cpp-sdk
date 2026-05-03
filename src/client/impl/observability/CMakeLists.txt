add_subdirectory(error_category)

_ydb_sdk_add_library(impl-observability)

target_link_libraries(impl-observability PUBLIC
  yutil
  client-metrics
  client-trace
  client-impl-ydb_stats
  impl-observability-error_category
  impl-internal-db_driver_state
)

target_sources(impl-observability PRIVATE
  metrics.cpp
  observation.cpp
  operation_name.cpp
  span.cpp
)

_ydb_sdk_install_targets(TARGETS impl-observability)