add_subdirectory(oauth2_token_exchange)

add_ydb_test(NAME client-ydb_coordination-ut
  SOURCES
  coordination_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Coordination
  api-grpc
)

add_ydb_test(NAME client-extensions-discovery_mutator-ut
  SOURCES
  discovery_mutator_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::DiscoveryMutator
  YDB-CPP-SDK::Table
)

add_ydb_test(NAME client-ydb_driver-ut
  SOURCES
  driver_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Core
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
  SOURCES
  params_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Params
  YDB-CPP-SDK::YsonValue
)

add_ydb_test(NAME client-ydb_result-ut
  SOURCES
  result_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Result
  YDB-CPP-SDK::Params
)

add_ydb_test(NAME client-ydb_value-ut
  SOURCES
  value_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Value
  YDB-CPP-SDK::JsonValue
  YDB-CPP-SDK::YsonValue
  YDB-CPP-SDK::Params
)

add_ydb_test(NAME client-draft-ut
  SOURCES
  ydb_scripting_response_headers_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Draft
)
