add_library(cpp-client-ydb_result)

target_link_libraries(cpp-client-ydb_result PUBLIC
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
  cpp-client-ydb_value
  cpp-client-ydb_proto
)

target_sources(cpp-client-ydb_result PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/result/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/result/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/result/CMakeLists.txt
  proto_accessor.cpp
  result.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result/proto_accessor.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result/result.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_result/CMakeLists.txt
========
  proto_accessor.cpp
  result.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_result/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_result/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  proto_accessor.cpp
  result.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_result/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::Result ALIAS cpp-client-ydb_result)
