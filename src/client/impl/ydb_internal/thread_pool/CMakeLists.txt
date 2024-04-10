add_library(impl-ydb_internal-thread_pool)

target_link_libraries(impl-ydb_internal-thread_pool PUBLIC
  yutil
)

target_sources(impl-ydb_internal-thread_pool PRIVATE
<<<<<<< HEAD:src/client/impl/ydb_internal/thread_pool/CMakeLists.txt
  pool.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/thread_pool/pool.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/thread_pool/CMakeLists.txt
)
