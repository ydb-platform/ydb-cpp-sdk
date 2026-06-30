add_ydb_test(NAME iam_metadata_it GTEST
  SOURCES
    iam_test_fixture.cpp
    metadata_it.cpp
  LINK_LIBRARIES
    tests-iam-mocks
    YDB-CPP-SDK::Iam
    YDB-CPP-SDK::Query
  LABELS
    integration
)

add_ydb_test(NAME iam_driver_auth_it GTEST
  SOURCES
    iam_test_fixture.cpp
    driver_auth_it.cpp
  LINK_LIBRARIES
    tests-iam-mocks
    YDB-CPP-SDK::Iam
    YDB-CPP-SDK::Query
  LABELS
    integration
)

add_ydb_test(NAME iam_jwt_it GTEST
  SOURCES
    iam_test_fixture.cpp
    jwt_it.cpp
  LINK_LIBRARIES
    tests-iam-mocks
    YDB-CPP-SDK::Iam
    YDB-CPP-SDK::Query
  LABELS
    integration
)

add_ydb_test(NAME iam_oauth_it GTEST
  SOURCES
    iam_test_fixture.cpp
    oauth_it.cpp
  LINK_LIBRARIES
    tests-iam-mocks
    YDB-CPP-SDK::Iam
    YDB-CPP-SDK::Query
  LABELS
    integration
)
