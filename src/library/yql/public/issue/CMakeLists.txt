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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/yql/public/issue/CMakeLists.txt
  yql_issue.cpp
  yql_issue_message.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue/yql_issue.cpp
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue/yql_issue_message.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/yql/public/issue/CMakeLists.txt
=======
  yql_issue.cpp
  yql_issue_message.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/yql/public/issue/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  yql_issue.cpp
  yql_issue_message.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  yql_issue.cpp
  yql_issue_message.cpp
>>>>>>> origin/main
)


