_ydb_sdk_add_library(client-types-executor)

target_sources(client-types-executor PRIVATE
  executor.cpp
)

target_link_libraries(client-types-executor PUBLIC
  yutil
  impl-internal-thread_pool
  impl-executor
  client-types-exceptions
)

_ydb_sdk_install_targets(TARGETS client-types-executor)
