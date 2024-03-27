add_library(cpp-client-ydb_extension)

target_link_libraries(cpp-client-ydb_extension PUBLIC
  yutil
  cpp-monlib-metrics
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_extension PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_extension/extension.cpp
)

add_library(YDB-CPP-SDK::Extension ALIAS cpp-client-ydb_extension)
