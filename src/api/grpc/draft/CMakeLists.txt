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
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/dummy.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/fq_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/ydb_clickhouse_internal_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/ydb_persqueue_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/ydb_datastreams_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/ydb_maintenance_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/ydb_logstore_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/draft/ydb_dynamic_config_v1.proto
)
target_proto_addincls(api-grpc-draft
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(api-grpc-draft
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(api-grpc-draft
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


