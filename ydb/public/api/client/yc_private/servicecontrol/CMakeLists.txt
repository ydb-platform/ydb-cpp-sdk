add_library(client-yc_private-servicecontrol)
set_property(TARGET client-yc_private-servicecontrol PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-servicecontrol PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-servicecontrol PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/servicecontrol/access_service.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/servicecontrol/resource.proto
)
target_proto_addincls(client-yc_private-servicecontrol
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_private-servicecontrol
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-servicecontrol
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


