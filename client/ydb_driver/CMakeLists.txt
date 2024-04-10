add_library(cpp-client-ydb_driver)

target_link_libraries(cpp-client-ydb_driver PUBLIC
  
  yutil
  impl-ydb_internal-common
  impl-ydb_internal-grpc_connections
  cpp-client-resources
  cpp-client-ydb_common_client
  client-ydb_types-status
)

target_sources(cpp-client-ydb_driver PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/ydb_driver/driver.cpp
)

add_library(YDB-CPP-SDK::Driver ALIAS cpp-client-ydb_driver)
