add_ydb_test(NAME json_value-ut
  SOURCES
  ydb_json_value_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::JsonValue
  library-json
  cpp-testing-unittest
  YDB-CPP-SDK::Proto
  YDB-CPP-SDK::Params
)
