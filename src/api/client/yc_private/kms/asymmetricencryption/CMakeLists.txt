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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/api/client/yc_private/kms/asymmetricencryption/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/kms/asymmetricencryption/asymmetric_encryption_key.proto
)
target_proto_addincls(yc_private-kms-asymmetricencryption
  ./
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/kms/asymmetricencryption/asymmetric_encryption_key.proto
)
target_proto_addincls(yc_private-kms-asymmetricencryption
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/kms/asymmetricencryption/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/kms/asymmetricencryption/asymmetric_encryption_key.proto
)
target_proto_addincls(yc_private-kms-asymmetricencryption
  ./
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/kms/asymmetricencryption/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/kms/asymmetricencryption/asymmetric_encryption_key.proto
)
target_proto_addincls(yc_private-kms-asymmetricencryption
  ./
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(yc_private-kms-asymmetricencryption
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(yc_private-kms-asymmetricencryption
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


