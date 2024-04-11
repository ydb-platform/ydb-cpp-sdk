add_subdirectory(protos)

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
  yql_issue.cpp
  yql_issue_message.cpp
)


