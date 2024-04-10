add_subdirectory(impl)

add_library(cpp-client-ydb_common_client)

target_link_libraries(cpp-client-ydb_common_client PUBLIC
  yutil
  impl-ydb_internal-common
  client-ydb_types-credentials
)

target_sources(cpp-client-ydb_common_client PRIVATE
<<<<<<< HEAD:src/client/common_client/CMakeLists.txt
  settings.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_common_client/settings.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_common_client/CMakeLists.txt
)

add_library(YDB-CPP-SDK::CommonClient ALIAS cpp-client-ydb_common_client)
