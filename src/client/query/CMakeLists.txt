add_subdirectory(impl)

_ydb_sdk_add_library(client-ydb_query)

# Work around LLVM 20 ThinLTO codegen crashes in transaction precommit.
set_target_properties(client-ydb_query PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
target_compile_options(client-ydb_query PRIVATE
  "$<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,GNU>:-fno-lto>")

target_link_libraries(client-ydb_query PUBLIC
  yutil
  impl-internal-make_request
  impl-session
  impl-internal-retry
  client-ydb_common_client
  client-ydb_driver
  client-ydb_query-impl
  client-ydb_result
  client-ydb_row_ranges
  client-metrics
  client-trace
  client-types-operation
  api-protos
  api-grpc
)

target_sources(client-ydb_query PRIVATE
  client.cpp
  query.cpp
  stats.cpp
  tx.cpp
)

_ydb_sdk_make_client_component(Query client-ydb_query)
