add_library(client-yc_public-logging)
set_property(TARGET client-yc_public-logging PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_public-logging PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  client-yc_public-common
  protobuf::libprotobuf
)
target_proto_messages(client-yc_public-logging PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/api/client/yc_public/logging/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_entry.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_ingestion_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_resource.proto
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/logging/log_entry.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/logging/log_ingestion_service.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_public/logging/log_resource.proto
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_public/logging/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_entry.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_ingestion_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_resource.proto
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_public/logging/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_entry.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_ingestion_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_resource.proto
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_entry.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_ingestion_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_public/logging/log_resource.proto
>>>>>>> origin/main
)
target_proto_addincls(client-yc_public-logging
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_public-logging
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_public-logging
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


