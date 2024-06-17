add_library(client-extensions-solomon_stats)

target_link_libraries(client-extensions-solomon_stats PUBLIC
  yutil
  monlib-encode-json
  monlib-metrics
  monlib-service
  monlib-service-pages
  client-ydb_extension
)

target_sources(client-extensions-solomon_stats PRIVATE
  pull_client.cpp
  pull_connector.cpp
)

_ydb_sdk_install_targets(TARGETS client-extensions-solomon_stats)

add_library(YDB-CPP-SDK::SolomonStats ALIAS client-extensions-solomon_stats)
