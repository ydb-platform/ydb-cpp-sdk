add_ydb_test(NAME server_restart GTEST
  SOURCES
    main.cpp
  LINK_LIBRARIES
    yutil
    api-grpc
    YDB-CPP-SDK::Query
    gRPC::grpc++
  LABELS
    integration
)
