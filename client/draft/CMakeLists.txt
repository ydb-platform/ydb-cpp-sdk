# add_subdirectory(ut)

add_library(cpp-client-draft)

target_link_libraries(cpp-client-draft PUBLIC
  yutil
  api-grpc-draft
  cpp-client-ydb_table
  client-ydb_types-operation
  cpp-client-ydb_value
)
target_sources(cpp-client-draft PRIVATE
  ${CMAKE_SOURCE_DIR}/client/draft/ydb_dynamic_config.cpp
  ${CMAKE_SOURCE_DIR}/client/draft/ydb_scripting.cpp
  ${CMAKE_SOURCE_DIR}/client/draft/ydb_long_tx.cpp
)

add_library(YDB-CPP-SDK::Draft ALIAS cpp-client-draft)
