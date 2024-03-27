add_subdirectory(impl)

add_library(cpp-client-ydb_common_client)

target_link_libraries(cpp-client-ydb_common_client PUBLIC
  yutil
  impl-ydb_internal-common
  client-ydb_types-credentials
)

target_sources(cpp-client-ydb_common_client PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_common_client/settings.cpp
)

add_library(YDB-CPP-SDK::CommonClient ALIAS cpp-client-ydb_common_client)
