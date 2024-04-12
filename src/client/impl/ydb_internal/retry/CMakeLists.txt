add_library(impl-ydb_internal-retry)

target_link_libraries(impl-ydb_internal-retry PUBLIC
  yutil
  impl-ydb_internal-grpc_connections
)

target_sources(impl-ydb_internal-retry PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/client/impl/ydb_internal/retry/CMakeLists.txt
  retry.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/retry/retry.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/retry/CMakeLists.txt
=======
  retry.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
