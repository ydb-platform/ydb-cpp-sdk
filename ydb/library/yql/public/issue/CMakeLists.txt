add_subdirectory(protos)
add_subdirectory(ut)

add_library(yql-public-issue)

target_link_libraries(yql-public-issue PUBLIC
  yutil
  protobuf::libprotobuf
  library-cpp-colorizer
  library-cpp-resource
  api-protos
  public-issue-protos
  library-yql-utils
  tools-enum_parser-enum_serialization_runtime
  cpp-string_utils-misc
)

target_sources(yql-public-issue PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue/yql_issue.cpp
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue/yql_issue_message.cpp
)


