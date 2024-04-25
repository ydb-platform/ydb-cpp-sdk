add_library(impl-ydb_internal-grpc_connections)

target_link_libraries(impl-ydb_internal-grpc_connections PUBLIC
  yutil
  api-grpc
  api-protos
  impl-ydb_internal-db_driver_state
  impl-ydb_internal-plain_status
  impl-ydb_internal-thread_pool
  client-impl-ydb_stats
  cpp-client-resources
  client-ydb_types-exceptions
)

target_sources(impl-ydb_internal-grpc_connections PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/impl/ydb_internal/grpc_connections/CMakeLists.txt
  actions.cpp
  grpc_connections.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/grpc_connections/actions.cpp
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/grpc_connections/grpc_connections.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/grpc_connections/CMakeLists.txt
=======
  actions.cpp
  grpc_connections.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/grpc_connections/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  actions.cpp
  grpc_connections.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  actions.cpp
  grpc_connections.cpp
>>>>>>> origin/main
)
