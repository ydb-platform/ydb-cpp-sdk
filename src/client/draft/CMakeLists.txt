add_library(cpp-client-draft)

target_link_libraries(cpp-client-draft PUBLIC
  yutil
  yql-public-issue
  api-grpc-draft
  cpp-client-ydb_table
  client-ydb_types-operation
  cpp-client-ydb_value
)

target_sources(cpp-client-draft PRIVATE
  ydb_dynamic_config.cpp
  ydb_replication.cpp
  ydb_scripting.cpp
)

generate_enum_serilization(cpp-client-draft
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/draft/ydb_replication.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/draft/ydb_replication.h
)

add_library(YDB-CPP-SDK::Draft ALIAS cpp-client-draft)
