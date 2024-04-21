add_subdirectory(draft)

add_library(api-grpc)
set_property(TARGET api-grpc PROPERTY
  PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
)
target_link_libraries(api-grpc PUBLIC
  yutil
  gRPC::grpc++
  api-protos
  protobuf::libprotobuf
)
target_proto_messages(api-grpc PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_federation_discovery_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_auth_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_cms_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_coordination_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_discovery_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_export_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_import_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_monitoring_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_operation_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_query_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_rate_limiter_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_scheme_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_scripting_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_table_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_topic_v1.proto
  ${YDB_SDK_SOURCE_DIR}/src/api/grpc/ydb_keyvalue_v1.proto
)
target_proto_addincls(api-grpc
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(api-grpc
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)
target_proto_plugin(api-grpc
  grpc_cpp
  gRPC::grpc_cpp_plugin
)


