add_ydb_test(NAME pagination_it
  SOURCES
  main.cpp
  pagination_data.cpp
  pagination.cpp
  pagination.h
  LINK_LIBRARIES
  yutil
  YDB-CPP-SDK::Table
  GTest::gtest_main
  public-lib-json_value
  LABELS
  integration
)