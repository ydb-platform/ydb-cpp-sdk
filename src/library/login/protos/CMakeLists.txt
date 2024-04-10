add_library(library-login-protos)

target_link_libraries(library-login-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(library-login-protos PRIVATE
<<<<<<< HEAD:src/library/login/protos/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/login/protos/login.proto
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/login/protos/login.proto
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/login/protos/CMakeLists.txt
)

target_proto_addincls(library-login-protos
  ./
<<<<<<< HEAD:src/library/login/protos/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/
  ${YDB_SDK_BINARY_DIR}
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/login/protos/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)

target_proto_outs(library-login-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)


