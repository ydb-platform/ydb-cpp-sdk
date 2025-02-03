add_ydb_test(NAME server_restart
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    api-grpc
    YDB-CPP-SDK::Query
    gRPC::grpc++
    GTest::gtest_main
  LABELS
    integration
)
