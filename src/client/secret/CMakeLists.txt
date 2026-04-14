_ydb_sdk_add_library(client-secret)

target_link_libraries(client-secret PUBLIC
  api-grpc
  impl-internal-make_request
  client-ydb_common_client-impl
  client-ydb_driver
  client-ydb_scheme
)

target_sources(client-secret PRIVATE
  secret.cpp
  out.cpp
)

_ydb_sdk_make_client_component(Secret client-secret)
