add_library(client-impl-ydb_stats)

target_link_libraries(client-impl-ydb_stats PUBLIC
  yutil
  library-grpc-client
  cpp-monlib-metrics
)

target_sources(client-impl-ydb_stats PRIVATE
  stats.cpp
)


