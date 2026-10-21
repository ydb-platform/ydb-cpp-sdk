_ydb_sdk_add_library(client-ydb_query-impl)

# Work around LLVM 20 ThinLTO codegen crashes in StreamExecuteQuery.
set_target_properties(client-ydb_query-impl PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
target_compile_options(client-ydb_query-impl PRIVATE
  "$<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,GNU>:-fno-lto>")

target_link_libraries(client-ydb_query-impl PUBLIC
  yutil
  api-grpc
  api-protos
  client-ydb_common_client-impl
  client-ydb_proto
  client-ydb_result
)

target_link_libraries(client-ydb_query-impl PUBLIC
  impl-observability
)

target_sources(client-ydb_query-impl PRIVATE
  exec_query.cpp
  client_session.cpp
  session_state_handler.cpp
)

_ydb_sdk_install_targets(TARGETS client-ydb_query-impl)
