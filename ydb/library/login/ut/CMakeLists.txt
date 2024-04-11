add_ydb_test(NAME library-login-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/ydb/library/login
  SOURCES
  ${YDB_SDK_SOURCE_DIR}/ydb/library/login/login_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  ydb-library-login
)
