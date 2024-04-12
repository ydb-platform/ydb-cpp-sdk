add_library(cpp-client-draft)

target_link_libraries(cpp-client-draft PUBLIC
  yutil
  api-grpc-draft
  cpp-client-ydb_table
  client-ydb_types-operation
  cpp-client-ydb_value
)

target_sources(cpp-client-draft PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/client/draft/CMakeLists.txt
  ydb_dynamic_config.cpp
  ydb_scripting.cpp
  ydb_long_tx.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/draft/ydb_dynamic_config.cpp
  ${YDB_SDK_SOURCE_DIR}/client/draft/ydb_scripting.cpp
  ${YDB_SDK_SOURCE_DIR}/client/draft/ydb_long_tx.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/draft/CMakeLists.txt
=======
  ydb_dynamic_config.cpp
  ydb_scripting.cpp
  ydb_long_tx.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::Draft ALIAS cpp-client-draft)
