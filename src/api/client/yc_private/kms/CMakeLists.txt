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
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/api/client/yc_private/kms/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/kms/symmetric_crypto_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/kms/symmetric_key.proto
)
target_proto_addincls(client-yc_private-kms
  ./
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/kms/symmetric_crypto_service.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/kms/symmetric_key.proto
)
target_proto_addincls(client-yc_private-kms
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/kms/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/kms/symmetric_crypto_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/kms/symmetric_key.proto
)
target_proto_addincls(client-yc_private-kms
  ./
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/kms/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(client-yc_private-kms
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-kms
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


