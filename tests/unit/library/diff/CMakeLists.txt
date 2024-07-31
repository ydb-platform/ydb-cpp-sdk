add_ydb_test(NAME diff-ut
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/diff
  LINK_LIBRARIES
    diff
    cpp-testing-unittest_main
  LABELS
    unit
)
