add_library(library-login-protos)

target_link_libraries(library-login-protos PUBLIC
  contrib-libs-cxxsupp
  yutil
  protobuf::libprotobuf
)

target_proto_messages(library-login-protos PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/login/protos/login.proto
)

target_proto_addincls(library-login-protos
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

target_proto_outs(library-login-protos
  --cpp_out=${CMAKE_BINARY_DIR}/
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-login-protos PUBLIC
    contrib-libs-linux-headers
  )
endif()
