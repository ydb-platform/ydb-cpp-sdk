_ydb_sdk_add_library(client-cms)

target_link_libraries(client-cms
  PUBLIC
    client-ydb_driver
  PRIVATE
    api-grpc
    api-protos
    client-ydb_common_client-impl
)

target_sources(client-cms
  PRIVATE
    cms.cpp
)

_ydb_sdk_make_client_component(Cms client-cms)
