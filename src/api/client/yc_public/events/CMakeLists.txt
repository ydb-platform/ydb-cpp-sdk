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
<<<<<<< HEAD
<<<<<<< HEAD:src/api/client/yc_public/events/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/events/options.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/events/common.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/events/yq.proto
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/events/options.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/events/common.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/events/yq.proto
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_public/events/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/events/options.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/events/common.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/events/yq.proto
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
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


