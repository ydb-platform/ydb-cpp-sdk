add_ydb_test(NAME metrics_it GTEST
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Query
    client-metrics
    impl-observability-error_category
  LABELS
    integration
)