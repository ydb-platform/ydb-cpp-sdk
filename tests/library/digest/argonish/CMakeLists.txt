add_ydb_test(NAME library-digest-argonish-ut
  SOURCES
    ut.cpp
  LINK_LIBRARIES
    digest-argonish
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-digest-argonish-fat_ut
  SOURCES
    fat_ut.cpp
  LINK_LIBRARIES
    digest-argonish
    cpp-testing-unittest_main
)
