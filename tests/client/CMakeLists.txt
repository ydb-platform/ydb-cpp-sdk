add_ydb_test(NAME client-ydb_value-ut
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/client/ydb_value
  SOURCES
  value_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Value
  public-lib-json_value
  public-lib-yson_value
  YDB-CPP-SDK::Params
)