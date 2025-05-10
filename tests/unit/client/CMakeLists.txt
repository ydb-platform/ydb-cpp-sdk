add_subdirectory(oauth2_token_exchange/helpers)

add_ydb_test(NAME client-ydb_coordination_ut
  SOURCES
    coordination/coordination_ut.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Coordination
    api-grpc
  LABELS
    unit
)

add_ydb_test(NAME client-extensions-discovery_mutator_ut
  SOURCES
    discovery_mutator/discovery_mutator_ut.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::DiscoveryMutator
    ydb-cpp-sdk::Table
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_driver_ut
  SOURCES
    driver/driver_ut.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Driver
    ydb-cpp-sdk::Table
  LABELS
    unit
)

add_ydb_test(NAME client-impl-ydb_endpoints_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
  SOURCES
    endpoints/endpoints_ut.cpp
  LINK_LIBRARIES
    yutil
    client-impl-ydb_endpoints
  LABELS
    unit
)

add_ydb_test(NAME client-oauth2_ut
  SOURCES
    oauth2_token_exchange/credentials_ut.cpp
    oauth2_token_exchange/jwt_token_source_ut.cpp
  LINK_LIBRARIES
    yutil
    http-server
    json
    string_utils-base64
    client-ydb_types-credentials-oauth2
    client-oauth2-ut-helpers
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_params_ut GTEST
  SOURCES
    params/params_ut.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Params
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_result_ut
  SOURCES
    result/result_ut.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Result
    ydb-cpp-sdk::Params
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_value_ut GTEST
  SOURCES
    value/value_ut.cpp
  LINK_LIBRARIES
    yutil
    ydb-cpp-sdk::Value
    ydb-cpp-sdk::Params
  LABELS
    unit
)
