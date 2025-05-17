add_ydb_test(NAME basic-example GTEST
  SOURCES
    main.cpp
    basic_example_data.cpp
    basic_example.cpp
  LINK_LIBRARIES
    yutil
    api-protos
    ydb-cpp-sdk::Driver
    ydb-cpp-sdk::Proto
    ydb-cpp-sdk::Table
  LABELS
    integration
)
