add_library(client-impl-ydb_endpoints)

target_link_libraries(client-impl-ydb_endpoints PUBLIC
  yutil
  cpp-monlib-metrics
  api-grpc
)

target_sources(client-impl-ydb_endpoints PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_endpoints/endpoints.cpp
)


