add_ydb_test(NAME sessions_pool_it
  SPLIT_FACTOR 12
  SOURCES
  sessions_pool_it.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Table
  api-grpc
  LABELS
  integration
)