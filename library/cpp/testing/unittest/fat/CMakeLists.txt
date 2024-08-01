add_ydb_test(NAME cpp-testing-unittest_fat_ut
  SOURCES
    test_port_manager.cpp
  LINK_LIBRARIES
    cpp-testing-unittest_main
  LABELS
    unit
)
