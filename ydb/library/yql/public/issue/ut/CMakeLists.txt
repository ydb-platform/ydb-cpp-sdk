add_ydb_test(NAME yql-public-issue-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue
  SOURCES
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/issue/yql_issue_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-issue
  cpp-unicode_normalization
)
