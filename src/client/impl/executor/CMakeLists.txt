_ydb_sdk_add_library(impl-executor)

target_sources(impl-executor PRIVATE
  executor_impl.cpp
)

target_link_libraries(impl-executor PUBLIC
  threading-local-executor
  yutil
)

_ydb_sdk_install_targets(TARGETS impl-executor)
