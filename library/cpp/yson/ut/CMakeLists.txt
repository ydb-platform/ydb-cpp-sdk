add_ydb_test(NAME yson-ut GTEST
  SOURCES
    yson_ut.cpp
  LINK_LIBRARIES
    yson
    GTest::gtest_main
  LABELS
    unit
)
