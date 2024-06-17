add_library(client-ydb_datastreams)

target_link_libraries(client-ydb_datastreams PUBLIC
  yutil
  library-grpc-client
  string_utils-url
  api-grpc-draft
  library-operation_id
  impl-ydb_internal-make_request
  client-ydb_driver
)

target_sources(client-ydb_datastreams PRIVATE
  datastreams.cpp
)

_ydb_sdk_install_targets(TARGETS client-ydb_datastreams)

add_library(YDB-CPP-SDK::Datastreams ALIAS client-ydb_datastreams)
