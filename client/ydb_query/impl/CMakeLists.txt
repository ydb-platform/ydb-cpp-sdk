add_library(client-ydb_query-impl)

target_link_libraries(client-ydb_query-impl PUBLIC
  yutil
  api-grpc-draft
  api-protos
  client-ydb_common_client-impl
  cpp-client-ydb_proto
)

target_sources(client-ydb_query-impl PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_query/impl/exec_query.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_query/impl/client_session.cpp
)


