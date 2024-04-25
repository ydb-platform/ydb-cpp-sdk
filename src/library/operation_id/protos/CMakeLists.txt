add_library(lib-operation_id-protos)
target_link_libraries(lib-operation_id-protos PUBLIC
  yutil
  protobuf::libprotobuf
)
target_proto_messages(lib-operation_id-protos PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/operation_id/protos/CMakeLists.txt
=======
>>>>>>> origin/main
  ${YDB_SDK_SOURCE_DIR}/src/library/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
<<<<<<< HEAD
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/lib/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/lib/operation_id/protos/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/lib/operation_id/protos/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)
target_proto_outs(lib-operation_id-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)

