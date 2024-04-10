add_library(client-ydb_table-query_stats)

target_link_libraries(client-ydb_table-query_stats PUBLIC
  yutil
  protobuf::libprotobuf
  api-protos
  cpp-client-ydb_query
)

target_sources(client-ydb_table-query_stats PRIVATE
<<<<<<< HEAD:src/client/table/query_stats/CMakeLists.txt
  stats.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/query_stats/stats.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_table/query_stats/CMakeLists.txt
)


