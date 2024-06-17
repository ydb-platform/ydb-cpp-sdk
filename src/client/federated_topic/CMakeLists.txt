add_subdirectory(impl)

add_library(cpp-client-ydb_federated_topic)

target_link_libraries(cpp-client-ydb_federated_topic PUBLIC
  yutil
  enum_serialization_runtime
  client-ydb_topic
  client-ydb_federated_topic-impl
)

generate_enum_serilization(cpp-client-ydb_federated_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/federated_topic/federated_topic.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/federated_topic/federated_topic.h
)

add_library(YDB-CPP-SDK::FederatedTopic ALIAS cpp-client-ydb_federated_topic)
