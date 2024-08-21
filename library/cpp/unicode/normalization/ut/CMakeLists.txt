add_ydb_test(NAME unicode-normalization-ut
  SOURCES
    normalization_ut.cpp
  LINK_LIBRARIES
    unicode-normalization
    cpp-testing-unittest_main
  LABELS
    unit
)
