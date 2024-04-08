add_library(cpp-retry-protos)

target_link_libraries(cpp-retry-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(cpp-retry-protos PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/retry/protos/retry_options.proto
)

target_proto_addincls(cpp-retry-protos
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

target_proto_outs(cpp-retry-protos
  --cpp_out=${CMAKE_BINARY_DIR}/
)

