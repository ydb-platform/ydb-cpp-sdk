add_ydb_test(NAME sessions_pool_ut
  SPLIT_FACTOR 12
  SOURCES
  sessions_pool_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Table
  api-grpc
  LABELS
  unit
)