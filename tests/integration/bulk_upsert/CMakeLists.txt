add_ydb_test(NAME bulk_upsert
  SOURCES
    main.cpp
    bulk_upsert.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Table
    GTest::gtest_main
  LABELS
    integration
)
