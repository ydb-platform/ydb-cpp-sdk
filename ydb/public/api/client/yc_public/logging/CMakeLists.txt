add_library(client-yc_public-logging)
set_property(TARGET client-yc_public-logging PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(client-yc_public-logging PUBLIC
  contrib-libs-googleapis-common-protos
  contrib-libs-cxxsupp
  yutil
  gRPC::grpc++
  client-yc_public-common
  protobuf::libprotobuf
)
target_proto_messages(client-yc_public-logging PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_public/logging/log_entry.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_public/logging/log_ingestion_service.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/client/yc_public/logging/log_resource.proto
)
target_proto_addincls(client-yc_public-logging
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/libs/googleapis-common-protos
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(client-yc_public-logging
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(client-yc_public-logging
  grpc_cpp
  gRPC::grpc_cpp_plugin
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-yc_public-logging PUBLIC
    contrib-libs-linux-headers
  )
endif()
