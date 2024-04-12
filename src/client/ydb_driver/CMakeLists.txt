add_library(cpp-client-ydb_driver)

target_link_libraries(cpp-client-ydb_driver PUBLIC
  yutil
  impl-ydb_internal-common
  impl-ydb_internal-grpc_connections
  cpp-client-resources
  cpp-client-ydb_common_client
  client-ydb_types-status
)

target_sources(cpp-client-ydb_driver PRIVATE
<<<<<<<< HEAD:src/client/driver/CMakeLists.txt
<<<<<<< HEAD:src/client/driver/CMakeLists.txt
  driver.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_driver/driver.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_driver/CMakeLists.txt
========
  driver.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_driver/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Driver ALIAS cpp-client-ydb_driver)
