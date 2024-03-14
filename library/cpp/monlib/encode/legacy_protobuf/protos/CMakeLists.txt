add_library(encode-legacy_protobuf-protos)

target_link_libraries(encode-legacy_protobuf-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(encode-legacy_protobuf-protos PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/legacy_protobuf/protos/metric_meta.proto
)

target_proto_addincls(encode-legacy_protobuf-protos
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

target_proto_outs(encode-legacy_protobuf-protos
  --cpp_out=${CMAKE_BINARY_DIR}/
)


