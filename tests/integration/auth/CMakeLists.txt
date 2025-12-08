add_ydb_test(NAME auth_it GTEST
  SOURCES
    login_it.cpp
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Credentials
    client-ydb_common_client
    jwt-cpp::jwt-cpp
  LABELS
    integration
)
