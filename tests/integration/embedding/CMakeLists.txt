add_ydb_test(NAME embedding_it GTEST
  SOURCES
    embedding_it.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Driver
    YDB-CPP-SDK::Params
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Value
  LABELS
    integration
)
