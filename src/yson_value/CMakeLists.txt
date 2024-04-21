add_library(public-lib-yson_value)

target_link_libraries(public-lib-yson_value PUBLIC
  yutil
  library-cpp-yson
  cpp-yson-node
  cpp-client-ydb_result
  cpp-client-ydb_value
  library-uuid
)

target_sources(public-lib-yson_value PRIVATE
  ydb_yson_value.cpp
)

add_library(YDB-CPP-SDK::YsonValue ALIAS public-lib-yson_value)
