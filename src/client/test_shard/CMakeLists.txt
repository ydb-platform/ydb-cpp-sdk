_ydb_sdk_add_library(client-ydb_test_shard)

target_link_libraries(client-ydb_test_shard PUBLIC
  yutil
  api-grpc
  impl-internal-make_request
  client-ydb_common_client-impl
  client-ydb_driver
  client-types-status
)

target_sources(client-ydb_test_shard PRIVATE
  test_shard.cpp
)

_ydb_sdk_make_client_component(TestShard client-ydb_test_shard)
