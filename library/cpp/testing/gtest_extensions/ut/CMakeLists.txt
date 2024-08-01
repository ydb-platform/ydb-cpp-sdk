add_ydb_test(NAME cpp-testing-gtest_extensions_ut GTEST
  SOURCES
    gtest_extensions_ut.cpp
    probe_ut.cpp
  LINK_LIBRARIES
    cpp-testing-gtest_main
  LABELS
    unit
)
