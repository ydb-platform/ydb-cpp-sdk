add_library(cpp-client-ydb_result)

target_link_libraries(cpp-client-ydb_result PUBLIC
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
  cpp-client-ydb_value
  cpp-client-ydb_proto
)

target_sources(cpp-client-ydb_result PRIVATE
<<<<<<< HEAD:src/client/result/CMakeLists.txt
  proto_accessor.cpp
  result.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result/proto_accessor.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result/result.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_result/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Result ALIAS cpp-client-ydb_result)
