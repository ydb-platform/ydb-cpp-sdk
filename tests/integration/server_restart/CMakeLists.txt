add_ydb_test(NAME bulk_upsert_simple_it
  SOURCES
    main.cpp
    bulk_upsert.cpp
    bulk_upsert.h
  LINK_LIBRARIES
    yutil
    api-grpc
    YDB-CPP-SDK::Query
    gRPC::grpc++
    GTest::gtest_main
  LABELS
    integration
)
