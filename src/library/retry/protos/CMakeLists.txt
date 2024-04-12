add_library(cpp-retry-protos)

target_link_libraries(cpp-retry-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(cpp-retry-protos PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/retry/protos/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/protos/retry_options.proto
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/retry/protos/retry_options.proto
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/retry/protos/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/protos/retry_options.proto
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/retry/protos/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/protos/retry_options.proto
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

target_proto_addincls(cpp-retry-protos
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)

target_proto_outs(cpp-retry-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)

