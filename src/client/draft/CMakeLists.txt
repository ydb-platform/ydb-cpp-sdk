add_library(cpp-client-draft)

target_link_libraries(cpp-client-draft PUBLIC
  yutil
  api-grpc-draft
  cpp-client-ydb_table
  client-ydb_types-operation
  cpp-client-ydb_value
)

target_sources(cpp-client-draft PRIVATE
  ydb_dynamic_config.cpp
  ydb_scripting.cpp
  ydb_long_tx.cpp
)

add_library(YDB-CPP-SDK::Draft ALIAS cpp-client-draft)
