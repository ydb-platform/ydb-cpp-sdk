add_ydb_test(NAME key_conflict_it GTEST
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Table
  LABELS
    integration
)
