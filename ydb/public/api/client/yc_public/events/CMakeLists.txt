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
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/events/options.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/events/common.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/events/yq.proto
)
target_proto_addincls(client-yc_public-events
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_public-events
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_public-events
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


