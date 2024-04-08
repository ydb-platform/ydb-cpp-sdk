add_library(yc_private-kms-asymmetricsignature)
set_property(TARGET yc_private-kms-asymmetricsignature PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(yc_private-kms-asymmetricsignature PUBLIC
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(yc_private-kms-asymmetricsignature PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/kms/asymmetricsignature/asymmetric_signature_key.proto
)
target_proto_addincls(yc_private-kms-asymmetricsignature
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(yc_private-kms-asymmetricsignature
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(yc_private-kms-asymmetricsignature
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


