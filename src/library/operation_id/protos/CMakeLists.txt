add_library(lib-operation_id-protos)
target_link_libraries(lib-operation_id-protos PUBLIC
  yutil
  protobuf::libprotobuf
)
target_proto_messages(lib-operation_id-protos PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/operation_id/protos/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/lib/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/lib/operation_id/protos/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(lib-operation_id-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)

