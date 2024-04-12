add_library(public-lib-json_value)

target_link_libraries(public-lib-json_value PUBLIC
  yutil
  cpp-json-writer
  cpp-string_utils-base64
  cpp-client-ydb_result
  cpp-client-ydb_value
  library-uuid
)

target_sources(public-lib-json_value PRIVATE
  ydb_json_value.cpp
)

add_library(YDB-CPP-SDK::JsonValue ALIAS public-lib-json_value)
