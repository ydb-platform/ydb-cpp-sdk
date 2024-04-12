add_ydb_test(NAME library-yql-utils-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/library/yql/utils
  SOURCES
  utf8_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  library-yql-utils
)

add_ydb_test(NAME yql-public-issue-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/library/yql/public/issue
  SOURCES
  yql_issue_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-issue
  cpp-unicode_normalization
)
