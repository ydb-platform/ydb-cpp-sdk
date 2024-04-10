add_library(client-ydb_table-query_stats)

target_link_libraries(client-ydb_table-query_stats PUBLIC
  yutil
  protobuf::libprotobuf
  api-protos
  cpp-client-ydb_query
)

target_sources(client-ydb_table-query_stats PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/table/query_stats/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/table/query_stats/CMakeLists.txt
  stats.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/query_stats/stats.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_table/query_stats/CMakeLists.txt
========
  stats.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/query_stats/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_table/query_stats/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


