add_ydb_test(NAME cgiparam-ut
  SOURCES
    ut.cpp
  LINK_LIBRARIES
    cgiparam
    cpp-testing-unittest_main
  LABELS
    unit
)
