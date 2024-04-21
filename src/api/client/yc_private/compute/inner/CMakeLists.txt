add_library(yc_private-compute-inner)
set_property(TARGET yc_private-compute-inner PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(yc_private-compute-inner PUBLIC
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(yc_private-compute-inner PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/compute/inner/disk_service.proto
)
target_proto_addincls(yc_private-compute-inner
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(yc_private-compute-inner
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(yc_private-compute-inner
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


