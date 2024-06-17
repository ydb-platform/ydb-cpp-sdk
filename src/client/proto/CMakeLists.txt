add_library(client-ydb_proto)

target_link_libraries(client-ydb_proto PUBLIC
  yutil
  api-grpc
  api-grpc-draft
  api-protos
  lib-operation_id-protos
  client-ydb_params
  client-ydb_value
  yql-public-issue-protos
)

target_sources(client-ydb_proto PRIVATE
  accessor.cpp
)

_ydb_sdk_install_targets(TARGETS client-ydb_proto)

add_library(YDB-CPP-SDK::Proto ALIAS client-ydb_proto)
