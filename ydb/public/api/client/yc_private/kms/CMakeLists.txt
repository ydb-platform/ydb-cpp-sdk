add_subdirectory(asymmetricencryption)
add_subdirectory(asymmetricsignature)

add_library(client-yc_private-kms)
set_property(TARGET client-yc_private-kms PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-kms PUBLIC
  yutil
  gRPC::grpc++
  yc_private-kms-asymmetricencryption
  yc_private-kms-asymmetricsignature
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-kms PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/kms/symmetric_crypto_service.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/kms/symmetric_key.proto
)
target_proto_addincls(client-yc_private-kms
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(client-yc_private-kms
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-kms
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


