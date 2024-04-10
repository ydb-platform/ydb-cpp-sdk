add_subdirectory(impl)

add_library(cpp-client-ydb_federated_topic)

target_link_libraries(cpp-client-ydb_federated_topic PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-client-ydb_topic
  client-ydb_federated_topic-impl
)

generate_enum_serilization(cpp-client-ydb_federated_topic
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/federated_topic/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/federated_topic/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/federated_topic/federated_topic.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_federated_topic/federated_topic.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_federated_topic/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_federated_topic/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/federated_topic/federated_topic.h
========
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_federated_topic/federated_topic.h
  INCLUDE_HEADERS
  src/client/ydb_federated_topic/federated_topic.h
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_federated_topic/CMakeLists.txt
)

add_library(YDB-CPP-SDK::FederatedTopic ALIAS cpp-client-ydb_federated_topic)
