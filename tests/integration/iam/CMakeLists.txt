# IAM integration tests require a recipe with:
#   ENV(YDB_ENFORCE_USER_TOKEN_REQUIREMENT=true)
#   ENV(YDB_DEFAULT_CLUSTERADMIN=root@builtin)
# See ydb/public/sdk/cpp/tests/integration/iam/ya.make in the ydb repo.
# OSS CI runs these under the integration-iam workflow job.

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
