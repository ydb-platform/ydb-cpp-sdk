add_library(api-grpc-draft)
set_property(TARGET api-grpc-draft PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(api-grpc-draft PUBLIC
  yutil
  gRPC::grpc++
  api-protos
  protobuf::libprotobuf
)
target_proto_messages(api-grpc-draft PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/dummy.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/fq_v1.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/ydb_clickhouse_internal_v1.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/ydb_persqueue_v1.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/ydb_datastreams_v1.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/ydb_long_tx_v1.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/ydb_maintenance_v1.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/ydb_logstore_v1.proto
  ${CMAKE_SOURCE_DIR}/ydb/public/api/grpc/draft/ydb_dynamic_config_v1.proto
)
target_proto_addincls(api-grpc-draft
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(api-grpc-draft
  --cpp_out=${CMAKE_BINARY_DIR}/
)
target_proto_plugin(api-grpc-draft
  grpc_cpp
  gRPC::grpc_cpp_plugin
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(api-grpc-draft PUBLIC
    
  )
endif()
