add_ydb_test(NAME colorizer-ut
  SOURCES
    colorizer_ut.cpp
  LINK_LIBRARIES
    colorizer
    cpp-testing-unittest_main
  LABELS
    unit
)
