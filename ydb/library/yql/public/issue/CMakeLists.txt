add_subdirectory(protos)

get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

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
  ${CMAKE_SOURCE_DIR}/ydb/library/yql/public/issue/yql_issue.cpp
  ${CMAKE_SOURCE_DIR}/ydb/library/yql/public/issue/yql_issue_message.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(yql-public-issue PUBLIC
    
  )
endif()
