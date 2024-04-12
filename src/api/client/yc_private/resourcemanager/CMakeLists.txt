add_library(client-yc_private-resourcemanager)
set_property(TARGET client-yc_private-resourcemanager PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_private-resourcemanager PUBLIC
  api-common-protos
  yutil
  gRPC::grpc++
  client-yc_private-operation
  client-yc_private-servicecontrol
  protobuf::libprotobuf
)
target_proto_messages(client-yc_private-resourcemanager PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/api/client/yc_private/resourcemanager/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/folder.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/transitional/folder_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/folder_service.proto
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/resourcemanager/folder.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/resourcemanager/transitional/folder_service.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/client/yc_private/resourcemanager/folder_service.proto
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/resourcemanager/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/folder.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/transitional/folder_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/folder_service.proto
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/api/client/yc_private/resourcemanager/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/folder.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/transitional/folder_service.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/client/yc_private/resourcemanager/folder_service.proto
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
target_proto_addincls(client-yc_private-resourcemanager
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos
)
target_proto_outs(client-yc_private-resourcemanager
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(client-yc_private-resourcemanager
  grpc_cpp
  gRPC::grpc_cpp_plugin
)
