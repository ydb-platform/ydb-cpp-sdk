add_ydb_test(NAME bulk_upsert
  SOURCES
    main.cpp
    bulk_upsert.cpp
    bulk_upsert.h
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Table
    GTest::gtest_main
  LABELS
    integration
)
