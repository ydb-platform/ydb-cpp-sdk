add_library(client-yc_private-servicecontrol)
set_property(TARGET client-yc_private-servicecontrol PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-servicecontrol PUBLIC
  contrib-libs-googleapis-common-protos
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-servicecontrol PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/servicecontrol/access_service.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/servicecontrol/resource.proto
)
target_proto_addincls(client-yc_private-servicecontrol
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/libs/googleapis-common-protos
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(client-yc_private-servicecontrol
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-servicecontrol
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


