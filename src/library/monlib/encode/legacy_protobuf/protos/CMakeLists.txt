add_library(encode-legacy_protobuf-protos)

target_link_libraries(encode-legacy_protobuf-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(encode-legacy_protobuf-protos PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/legacy_protobuf/protos/metric_meta.proto
)

target_proto_addincls(encode-legacy_protobuf-protos
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)

target_proto_outs(encode-legacy_protobuf-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)


