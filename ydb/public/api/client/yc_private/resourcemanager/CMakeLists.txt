add_library(client-yc_private-resourcemanager)
set_property(TARGET client-yc_private-resourcemanager PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-resourcemanager PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  client-yc_private-operation
  client-yc_private-servicecontrol
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-resourcemanager PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/resourcemanager/folder.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/resourcemanager/transitional/folder_service.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/resourcemanager/folder_service.proto
)
target_proto_addincls(client-yc_private-resourcemanager
  ./
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_private-resourcemanager
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-resourcemanager
  grpc_cpp
  gRPC::grpc_cpp_plugin
)
