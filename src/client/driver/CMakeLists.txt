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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/driver/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/driver/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/driver/CMakeLists.txt
  driver.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_driver/driver.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_driver/CMakeLists.txt
========
  driver.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_driver/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_driver/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  driver.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_driver/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  driver.cpp
>>>>>>> origin/main
)

add_library(YDB-CPP-SDK::Driver ALIAS cpp-client-ydb_driver)
