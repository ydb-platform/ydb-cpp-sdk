add_library(client-yc_private-iam)
set_property(TARGET client-yc_private-iam PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-iam PUBLIC
  contrib-libs-googleapis-common-protos
  contrib-libs-cxxsupp
  yutil
  gRPC::grpc++
  client-yc_private-operation
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-iam PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/iam_token_service_subject.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/iam_token_service.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/iam_token.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/oauth_request.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/reference.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/service_account_service.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/service_account.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/user_account_service.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/user_account.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/iam/yandex_passport_cookie.proto
)
target_proto_addincls(client-yc_private-iam
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/libs/googleapis-common-protos
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(client-yc_private-iam
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-iam
  grpc_cpp
  gRPC::grpc_cpp_plugin
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-yc_private-iam PUBLIC
    contrib-libs-linux-headers
  )
endif()
