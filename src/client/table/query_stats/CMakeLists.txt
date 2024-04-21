add_library(client-ydb_table-query_stats)

target_link_libraries(client-ydb_table-query_stats PUBLIC
  yutil
  protobuf::libprotobuf
  api-protos
  cpp-client-ydb_query
)

target_sources(client-ydb_table-query_stats PRIVATE
  stats.cpp
)


