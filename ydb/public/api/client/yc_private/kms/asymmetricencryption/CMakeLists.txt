add_library(yc_private-kms-asymmetricencryption)
set_property(TARGET yc_private-kms-asymmetricencryption PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(yc_private-kms-asymmetricencryption PUBLIC
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(yc_private-kms-asymmetricencryption PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_private/kms/asymmetricencryption/asymmetric_encryption_key.proto
)
target_proto_addincls(yc_private-kms-asymmetricencryption
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(yc_private-kms-asymmetricencryption
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(yc_private-kms-asymmetricencryption
  grpc_cpp
  gRPC::grpc_cpp_plugin
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(yc_private-kms-asymmetricencryption PUBLIC
    
  )
endif()
