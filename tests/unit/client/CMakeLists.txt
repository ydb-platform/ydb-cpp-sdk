add_subdirectory(oauth2_token_exchange/helpers)

add_ydb_test(NAME client-connection_string_ut GTEST
  SOURCES
    connection_string/connection_string_ut.cpp
  LINK_LIBRARIES
    impl-internal-common
  LABELS
    unit
)

add_ydb_test(NAME client-coordination_ut
  SOURCES
    coordination/coordination_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Coordination
    api-grpc
  LABELS
    unit
)

add_ydb_test(NAME client-extensions-discovery_mutator_ut
  SOURCES
    discovery_mutator/discovery_mutator_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::DiscoveryMutator
    YDB-CPP-SDK::Table
  LABELS
    unit
)

add_ydb_test(NAME client-driver_ut
  SOURCES
    driver/driver_ut.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Driver
    YDB-CPP-SDK::Table
  LABELS
    unit
)

add_ydb_test(NAME client-endpoints_ut
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

add_ydb_test(NAME client-iam_grpc_ut GTEST
  SOURCES
    iam/grpc_iam_ut.cpp
  LINK_LIBRARIES
    api-client-yc_public
    cpp-testing-common
    tests-iam-mocks
    YDB-CPP-SDK::Iam
  LABELS
    unit
)

add_ydb_test(NAME client-iam_http_ut GTEST
  SOURCES
    iam/http_iam_ut.cpp
  LINK_LIBRARIES
    http-server
    json
    cpp-testing-common
    tests-iam-mocks
    YDB-CPP-SDK::Iam
  LABELS
    unit
)

add_ydb_test(NAME client-oauth2_ut
  SOURCES
    oauth2_token_exchange/credentials_ut.cpp
    oauth2_token_exchange/jwt_token_source_ut.cpp
  LINK_LIBRARIES
    http-server
    json
    string_utils-base64
    client-types-credentials-oauth2
    client-oauth2-ut-helpers
  LABELS
    unit
)

add_ydb_test(NAME client-params_ut GTEST
  SOURCES
    params/params_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Params
  LABELS
    unit
)

add_ydb_test(NAME client-table_ut GTEST
  SOURCES
    table/table_ut.cpp
  LINK_LIBRARIES
    api-grpc
    cpp-testing-common
    YDB-CPP-SDK::Table
  LABELS
    unit
)

add_ydb_test(NAME client-result_ut
  SOURCES
    result/result_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Result
    YDB-CPP-SDK::Params
  LABELS
    unit
)

add_ydb_test(NAME client-value_ut GTEST
  SOURCES
    value/value_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Value
    YDB-CPP-SDK::Params
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_metrics_ut GTEST
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    observability/metrics_ut.cpp
  LINK_LIBRARIES
    yutil
    impl-observability
    client-ydb_query-impl
    client-ydb_table-impl
    client-metrics
  LABELS
    unit
)

add_ydb_test(NAME client-ydb_spans_ut GTEST
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    observability/spans_ut.cpp
  LINK_LIBRARIES
    yutil
    impl-observability
    client-ydb_query-impl
    client-ydb_table-impl
    client-metrics
    client-trace
  LABELS
    unit
)

add_ydb_test(NAME client-build_info_ut GTEST
  SOURCES
    build_info/build_info_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Driver
  LABELS
    unit
)

add_ydb_test(NAME client-query_session_ut
  SOURCES
    query/client_session_ut.cpp
  LINK_LIBRARIES
    client-ydb_query-impl
    impl-session
    library-operation_id
  LABELS
    unit
)

add_ydb_test(NAME client-query_stats_ut
  SOURCES
    query/query_stats_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Query
  LABELS
    unit
)

add_ydb_test(NAME client-row_ranges_ut
  SOURCES
    row_ranges/row_ranges_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Table
  LABELS
    unit
)

add_ydb_test(NAME client-row_ranges_public_include_ut
  SOURCES
    row_ranges/row_ranges_public_include_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Table
  LABELS
    unit
)

add_ydb_test(NAME client-retry_range_ut
  SOURCES
    retry_range/retry_range_ut.cpp
  LINK_LIBRARIES
    api-grpc
    grpc-client
    YDB-CPP-SDK::Driver
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Table
  LABELS
    unit
)

add_ydb_test(NAME client-iam_private_grpc_ut GTEST
  SOURCES
    iam_private/grpc_iam_service_ut.cpp
  LINK_LIBRARIES
    api-client-yc_private
    tests-iam-mocks
    YDB-CPP-SDK::IamPrivate
  LABELS
    unit
)

add_ydb_test(NAME client-metric_buffer_ut GTEST
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    observability/metric_buffer_ut.cpp
  LINK_LIBRARIES
    impl-observability
    client-metrics
  LABELS
    unit
)