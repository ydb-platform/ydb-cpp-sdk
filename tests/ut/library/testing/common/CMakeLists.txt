add_ydb_test(NAME cpp-testing-common_ut
  SOURCES
  env_ut.cpp
  network_ut.cpp
  scope_ut.cpp
  LINK_LIBRARIES
  cpp-testing-gtest_main
  LABELS
  unit
)
