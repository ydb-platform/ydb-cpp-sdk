add_library(client-yc_private-operation)
set_property(TARGET client-yc_private-operation PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-operation PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-operation PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/api/client/yc_private/operation/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/operation/operation.proto
)
target_proto_addincls(client-yc_private-operation
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/operation/operation.proto
<<<<<<< HEAD
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/operation/operation.proto
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
target_proto_addincls(client-yc_private-operation
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/operation/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
)
target_proto_addincls(client-yc_private-operation
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
  ${YDB_SDK_BINARY_DIR}
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/operation/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
target_proto_outs(client-yc_private-operation
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-operation
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


