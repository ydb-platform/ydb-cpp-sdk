add_ydb_test(NAME json_value-ut
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/ydb/public/lib/json_value
  SOURCES
  ydb_json_value_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  public-lib-json_value
  library-cpp-json
  cpp-testing-unittest
  YDB-CPP-SDK::Proto
  YDB-CPP-SDK::Params
)
