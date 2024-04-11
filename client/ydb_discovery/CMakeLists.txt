add_library(cpp-client-ydb_discovery)

target_compile_options(cpp-client-ydb_discovery PRIVATE
  -Wno-deprecated
)

target_link_libraries(cpp-client-ydb_discovery PUBLIC
  yutil
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_discovery PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/ydb_discovery/discovery.cpp
)

add_library(YDB-CPP-SDK::Discovery ALIAS cpp-client-ydb_discovery)
