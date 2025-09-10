_ydb_sdk_add_library(client-metrics_providers-monlib)

target_link_libraries(client-metrics_providers-monlib PUBLIC
  yutil
  monlib-metrics
)

target_sources(client-metrics_providers-monlib
  PRIVATE
    provider.cpp
)
