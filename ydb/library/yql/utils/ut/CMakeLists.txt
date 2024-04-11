add_ydb_test(NAME library-yql-utils-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/utils
  SOURCES
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/utils/utf8_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  library-yql-utils
)
