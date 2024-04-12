add_library(cpp-client-ydb_operation)

target_link_libraries(cpp-client-ydb_operation PUBLIC
  yutil
  api-grpc
  library-operation_id
  cpp-client-ydb_query
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_export
  cpp-client-ydb_import
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_operation PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/operation/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/operation/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/operation/CMakeLists.txt
  operation.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_operation/operation.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_operation/CMakeLists.txt
========
  operation.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_operation/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_operation/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  operation.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_operation/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::Operation ALIAS cpp-client-ydb_operation)
