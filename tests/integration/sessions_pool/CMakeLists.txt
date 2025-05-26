add_ydb_test(NAME sessions_pool_it GTEST
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Table
    api-grpc
  LABELS
    integration
)
