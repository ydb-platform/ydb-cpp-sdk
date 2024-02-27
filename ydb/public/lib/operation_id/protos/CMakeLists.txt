add_library(lib-operation_id-protos)
target_link_libraries(lib-operation_id-protos PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
  yutil
  protobuf::libprotobuf
)
target_proto_messages(lib-operation_id-protos PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/public/lib/operation_id/protos/operation_id.proto
)
target_proto_addincls(lib-operation_id-protos
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)
target_proto_outs(lib-operation_id-protos
  --cpp_out=${CMAKE_BINARY_DIR}/
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(lib-operation_id-protos PUBLIC
    contrib-libs-linux-headers
  )
endif()