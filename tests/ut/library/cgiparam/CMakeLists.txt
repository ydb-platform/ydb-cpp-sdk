add_ydb_test(NAME library-cgiparam_ut
  SOURCES
    ut.cpp
  LINK_LIBRARIES
    library-cgiparam
    cpp-testing-unittest_main
)
