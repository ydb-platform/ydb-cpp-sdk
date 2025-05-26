add_ydb_test(NAME sessions_it GTEST
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Table
    ydb-cpp-sdk::Query
    api-grpc
    grpc-client
  LABELS
    integration
)
