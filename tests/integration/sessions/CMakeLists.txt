add_ydb_test(NAME sessions_it
  SOURCES
  sessions_it.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Driver
  YDB-CPP-SDK::Table
  api-grpc
  LABELS
  integration
)