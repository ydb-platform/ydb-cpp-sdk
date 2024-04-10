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
<<<<<<< HEAD:src/client/types/status/CMakeLists.txt
  status.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/status/status.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/status/CMakeLists.txt
)


