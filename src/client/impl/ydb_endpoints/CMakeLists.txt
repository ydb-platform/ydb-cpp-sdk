add_library(client-impl-ydb_endpoints)

target_link_libraries(client-impl-ydb_endpoints PUBLIC
  yutil
  cpp-monlib-metrics
  api-grpc
)

target_sources(client-impl-ydb_endpoints PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/impl/ydb_endpoints/CMakeLists.txt
  endpoints.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_endpoints/endpoints.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_endpoints/CMakeLists.txt
=======
  endpoints.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_endpoints/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  endpoints.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


