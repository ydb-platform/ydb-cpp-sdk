add_library(cpp-retry-protos)

target_link_libraries(cpp-retry-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(cpp-retry-protos PRIVATE
<<<<<<< HEAD:src/library/retry/protos/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/protos/retry_options.proto
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/retry/protos/retry_options.proto
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/retry/protos/CMakeLists.txt
)

target_proto_addincls(cpp-retry-protos
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)

target_proto_outs(cpp-retry-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)

