add_ydb_test(NAME sessions_pool_it GTEST
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Table
    api-grpc
  LABELS
    integration
)
