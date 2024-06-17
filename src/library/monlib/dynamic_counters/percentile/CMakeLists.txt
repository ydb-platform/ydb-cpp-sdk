add_library(monlib-dynamic_counters-percentile INTERFACE)

target_link_libraries(monlib-dynamic_counters-percentile INTERFACE
  yutil
  containers-stack_vector
  monlib-dynamic_counters
)

_ydb_sdk_install_targets(TARGETS monlib-dynamic_counters-percentile)
