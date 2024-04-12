add_ydb_test(NAME library-login-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/library/login
  SOURCES
  login_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  library-login
)