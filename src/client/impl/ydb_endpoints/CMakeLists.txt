add_library(client-impl-ydb_endpoints)

target_link_libraries(client-impl-ydb_endpoints PUBLIC
  yutil
  cpp-monlib-metrics
  api-grpc
)

target_sources(client-impl-ydb_endpoints PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/client/impl/ydb_endpoints/CMakeLists.txt
  endpoints.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_endpoints/endpoints.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_endpoints/CMakeLists.txt
=======
  endpoints.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


