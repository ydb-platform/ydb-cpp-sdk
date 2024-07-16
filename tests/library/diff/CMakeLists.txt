add_ydb_test(NAME library-diff-ut
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/diff
  LINK_LIBRARIES
    library-diff
    cpp-testing-unittest_main
)
