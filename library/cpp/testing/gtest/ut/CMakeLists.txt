add_ydb_test(NAME cpp-testing-gtest_ut GTEST
  SOURCES
    matchers_ut.cpp
    ut.cpp
  LINK_LIBRARIES
    cpp-testing-gtest_main
  LABELS
    unit
)
