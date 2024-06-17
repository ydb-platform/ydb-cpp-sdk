add_library(client-ydb_driver)

target_link_libraries(client-ydb_driver PUBLIC
  yutil
  impl-ydb_internal-common
  impl-ydb_internal-grpc_connections
  client-resources
  client-ydb_common_client
  client-ydb_types-status
)

target_sources(client-ydb_driver PRIVATE
  driver.cpp
)

_ydb_sdk_install_targets(TARGETS client-ydb_driver)

add_library(YDB-CPP-SDK::Core ALIAS client-ydb_driver)
