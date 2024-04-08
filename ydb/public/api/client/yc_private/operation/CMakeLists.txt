add_library(client-yc_private-operation)
set_property(TARGET client-yc_private-operation PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-operation PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-operation PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/operation/operation.proto
)
target_proto_addincls(client-yc_private-operation
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(client-yc_private-operation
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-operation
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


