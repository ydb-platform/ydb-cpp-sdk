add_library(public-issue-protos)

target_link_libraries(public-issue-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(public-issue-protos PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/yql/public/issue/protos/issue_message.proto
  ${CMAKE_SOURCE_DIR}/ydb/library/yql/public/issue/protos/issue_severity.proto
)

target_proto_addincls(public-issue-protos
  ./
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

target_proto_outs(public-issue-protos
  --cpp_out=${CMAKE_BINARY_DIR}/
)


