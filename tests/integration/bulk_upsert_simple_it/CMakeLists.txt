add_ydb_test(NAME bulk_upsert_simple_it
  SOURCES
  main.cpp
  LINK_LIBRARIES
  yutil
  YDB-CPP-SDK::Table
  GTest::gtest_main
  public-lib-json_value
  LABELS
  integration
)
