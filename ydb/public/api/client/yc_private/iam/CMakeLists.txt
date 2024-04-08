add_library(client-yc_private-iam)
set_property(TARGET client-yc_private-iam PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-iam PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  client-yc_private-operation
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-iam PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/iam_token_service_subject.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/iam_token_service.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/iam_token.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/oauth_request.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/reference.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/service_account_service.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/service_account.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/user_account_service.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/user_account.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/yandex_passport_cookie.proto
)
target_proto_addincls(client-yc_private-iam
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_private-iam
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-iam
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


