add_library(cpp-client-ydb_datastreams)

target_link_libraries(cpp-client-ydb_datastreams PUBLIC
  yutil
  library-grpc-client
  cpp-string_utils-url
  api-grpc-draft
  library-operation_id
  impl-ydb_internal-make_request
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_datastreams PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/datastreams/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/datastreams/CMakeLists.txt
  datastreams.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_datastreams/datastreams.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_datastreams/CMakeLists.txt
========
  datastreams.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_datastreams/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_datastreams/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_library(YDB-CPP-SDK::Datastreams ALIAS cpp-client-ydb_datastreams)
