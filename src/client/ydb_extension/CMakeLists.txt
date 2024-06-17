add_library(client-ydb_extension)

target_link_libraries(client-ydb_extension PUBLIC
  yutil
  monlib-metrics
  client-ydb_driver
)

target_sources(client-ydb_extension PRIVATE
  extension.cpp
)

_ydb_sdk_install_targets(TARGETS client-ydb_extension)

add_library(YDB-CPP-SDK::Extension ALIAS client-ydb_extension)
