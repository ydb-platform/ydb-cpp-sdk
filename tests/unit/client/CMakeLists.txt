add_subdirectory(oauth2_token_exchange/helpers)

add_ydb_test(NAME client-ydb_coordination_ut
  SOURCES
    coordination/coordination_ut.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Coordination
    api-grpc
  LABELS
    unit
)

add_ydb_test(NAME client-extensions-discovery_mutator_ut
  SOURCES
    discovery_mutator/discovery_mutator_ut.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::DiscoveryMutator
    YDB-CPP-SDK::Table
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_driver_ut
  SOURCES
    driver/driver_ut.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Driver
    YDB-CPP-SDK::Table
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

add_ydb_test(NAME client-impl-internal-local_dc_detector_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/client/impl/internal/local_dc_detector
  SOURCES
    local_dc_detector/local_dc_detector_ut.cpp
  LINK_LIBRARIES
    yutil
    api-protos
    impl-internal-local_dc_detector
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
    client-types-credentials-oauth2
    client-oauth2-ut-helpers
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_params_ut GTEST
  SOURCES
    params/params_ut.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Params
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_result_ut
  SOURCES
    result/result_ut.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Result
    YDB-CPP-SDK::Params
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_value_ut GTEST
  SOURCES
    value/value_ut.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Value
    YDB-CPP-SDK::Params
  LABELS
    unit
)
