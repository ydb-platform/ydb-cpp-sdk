add_library(impl-ydb_internal-thread_pool)

target_link_libraries(impl-ydb_internal-thread_pool PUBLIC
  yutil
)

target_sources(impl-ydb_internal-thread_pool PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/thread_pool/pool.cpp
)


