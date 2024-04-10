add_library(client-yc_public-iam)
set_property(TARGET client-yc_public-iam PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_public-iam PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  client-yc_public-common
  protobuf::libprotobuf
)
target_proto_messages(client-yc_public-iam PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/api/client/yc_public/iam/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/iam/iam_token_service.proto
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/iam/iam_token_service.proto
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_public/iam/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/iam/iam_token_service.proto
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_public/iam/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
target_proto_addincls(client-yc_public-iam
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_public-iam
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_public-iam
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


