add_library(client-impl-ydb_stats)

target_link_libraries(client-impl-ydb_stats PUBLIC
  yutil
  library-grpc-client
  cpp-monlib-metrics
)

target_sources(client-impl-ydb_stats PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/client/impl/ydb_stats/CMakeLists.txt
  stats.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_stats/stats.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_stats/CMakeLists.txt
=======
  stats.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


