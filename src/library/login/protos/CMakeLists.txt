add_library(library-login-protos)

target_link_libraries(library-login-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(library-login-protos PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/login/protos/login.proto
)

target_proto_addincls(library-login-protos
  ./
  ${YDB_SDK_SOURCE_DIR}
  ${YDB_SDK_BINARY_DIR}
)

target_proto_outs(library-login-protos
  --cpp_out=${YDB_SDK_BINARY_DIR}/
)


