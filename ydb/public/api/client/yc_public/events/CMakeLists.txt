add_library(client-yc_public-events)
set_property(TARGET client-yc_public-events PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_public-events PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  client-yc_public-common
  protobuf::libprotobuf
)
target_proto_messages(client-yc_public-events PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_public/events/options.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_public/events/common.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_public/events/yq.proto
)
target_proto_addincls(client-yc_public-events
  ./
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_public-events
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_public-events
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


