add_library(client-ydb_table-query_stats)

target_link_libraries(client-ydb_table-query_stats PUBLIC
  yutil
  protobuf::libprotobuf
  api-protos
  cpp-client-ydb_query
)

target_sources(client-ydb_table-query_stats PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_table/query_stats/stats.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_table-query_stats PUBLIC
    
  )
endif()
