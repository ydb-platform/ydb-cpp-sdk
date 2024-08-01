add_ydb_test(NAME util-charset-ut
  SOURCES
    ../utf8_ut.cpp
    ../wide_ut.cpp
  LINK_LIBRARIES
    util-charset
    cpp-testing-unittest_main
  LABELS
    unit
)
