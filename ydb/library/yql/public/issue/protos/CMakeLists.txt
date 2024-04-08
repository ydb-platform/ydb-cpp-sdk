add_library(public-issue-protos)

target_link_libraries(public-issue-protos PUBLIC
  yutil
  protobuf::libprotobuf
)

target_proto_messages(public-issue-protos PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue/protos/issue_message.proto
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue/protos/issue_severity.proto
)

target_proto_addincls(public-issue-protos
  ./
  ${YDB_SDK_SOURCE_DIR}/
  ${CMAKE_BINARY_DIR}
  ${YDB_SDK_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

target_proto_outs(public-issue-protos
  --cpp_out=${CMAKE_BINARY_DIR}/
)


