add_ydb_test(NAME bulk_upsert_it GTEST
  SOURCES
    main.cpp
    bulk_upsert.cpp
    bulk_upsert.h
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Table
  LABELS
    integration
)
