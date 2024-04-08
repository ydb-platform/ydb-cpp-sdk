add_library(api-protos-annotations)
target_link_libraries(api-protos-annotations PUBLIC
  yutil
  protobuf::libprotobuf
)
target_proto_messages(api-protos-annotations PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/protos/annotations/sensitive.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/public/api/protos/annotations/validation.proto
)
target_proto_addincls(api-protos-annotations
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(api-protos-annotations
  --cpp_out=${CMAKE_BINARY_DIR}/
)


