add_library(cpp-client-ydb_proto)

target_link_libraries(cpp-client-ydb_proto PUBLIC
  yutil
  api-grpc
  api-grpc-draft
  api-protos
  lib-operation_id-protos
  cpp-client-ydb_params
  cpp-client-ydb_value
  public-issue-protos
)

target_sources(cpp-client-ydb_proto PRIVATE
  accessor.cpp
)

add_library(YDB-CPP-SDK::Proto ALIAS cpp-client-ydb_proto)
