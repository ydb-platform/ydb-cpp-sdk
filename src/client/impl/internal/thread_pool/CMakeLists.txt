_ydb_sdk_add_library(impl-internal-thread_pool)

target_link_libraries(impl-internal-thread_pool PUBLIC
  yutil
)

target_sources(impl-internal-thread_pool PRIVATE
  pool.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-thread_pool)
