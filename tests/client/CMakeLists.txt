add_ydb_test(NAME client-ydb_coordination-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_coordination
  SOURCES
  coordination_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Coordination
  api-grpc
)

add_ydb_test(NAME client-extensions-discovery_mutator-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/extensions/discovery_mutator
  SOURCES
  discovery_mutator_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::DiscoveryMutator
  YDB-CPP-SDK::Table
)

add_ydb_test(NAME client-ydb_driver-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_driver
  SOURCES
  driver_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Driver
  YDB-CPP-SDK::Table
)

add_ydb_test(NAME client-impl-ydb_endpoints-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
  SOURCES
  endpoints_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  client-impl-ydb_endpoints
)

add_ydb_test(NAME client-ydb_params-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_params
  SOURCES
  params_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Params
  public-lib-yson_value
)

add_ydb_test(NAME client-ydb_result-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_result
  SOURCES
  result_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Result
  YDB-CPP-SDK::Params
)

add_ydb_test(NAME client-ydb_value-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_value
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

add_ydb_test(NAME client-draft-ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/client/draft
  SOURCES
  ydb_scripting_response_headers_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Draft
)
