add_ydb_test(NAME sessions_it GTEST
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Table
    YDB-CPP-SDK::Query
    api-grpc
    grpc-client
  LABELS
    integration
)
