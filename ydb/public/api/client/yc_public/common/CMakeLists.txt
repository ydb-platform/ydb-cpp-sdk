add_library(client-yc_public-common)
set_property(TARGET client-yc_public-common PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_public-common PUBLIC
  contrib-libs-googleapis-common-protos
  yutil
  gRPC::grpc++
  protobuf::libprotobuf
)
target_proto_messages(client-yc_public-common PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_public/common/validation.proto
)
target_proto_addincls(client-yc_public-common
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/libs/googleapis-common-protos
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(client-yc_public-common
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_public-common
  grpc_cpp
  gRPC::grpc_cpp_plugin
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-yc_public-common PUBLIC
    
  )
endif()
