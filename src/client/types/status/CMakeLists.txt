add_library(client-ydb_types-status)

target_link_libraries(client-ydb_types-status PUBLIC
  yutil
  cpp-threading-future
  impl-ydb_internal-plain_status
  cpp-client-ydb_types
  client-ydb_types-fatal_error_handlers
  yql-public-issue
)

target_sources(client-ydb_types-status PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/types/status/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/types/status/CMakeLists.txt
  status.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/status/status.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/status/CMakeLists.txt
========
  status.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/status/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/status/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


