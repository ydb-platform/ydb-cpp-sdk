add_ydb_test(NAME bulk_upsert GTEST
  SOURCES
    main.cpp
    bulk_upsert.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Table
  LABELS
    integration
)
