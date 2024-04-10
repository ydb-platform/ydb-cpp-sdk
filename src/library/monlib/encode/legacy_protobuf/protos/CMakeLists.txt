add_library(encode-legacy_protobuf-protos)

target_link_libraries(encode-legacy_protobuf-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(encode-legacy_protobuf-protos PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/monlib/encode/legacy_protobuf/protos/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/legacy_protobuf/protos/metric_meta.proto
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/legacy_protobuf/protos/metric_meta.proto
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/legacy_protobuf/protos/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/legacy_protobuf/protos/metric_meta.proto
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/legacy_protobuf/protos/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

target_proto_addincls(encode-legacy_protobuf-protos
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)

target_proto_outs(encode-legacy_protobuf-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)


