# read_session_membership_ut.cpp requires the in-process YDB/PersQueue test
# server, which is not included in the standalone SDK.

add_ydb_test(NAME client-federated-topic-observer_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    ${YDB_SDK_SOURCE_DIR}/src/client/federated_topic/ut/federation_observer_deadlock_ut.cpp
  LINK_LIBRARIES
    YDB-CPP-SDK::FederatedTopic
    gRPC::grpc++
  LABELS
    unit
)
