_ydb_sdk_add_library(threading-local-executor)

target_sources(threading-local-executor PRIVATE
  tbb_local_executor.cpp
)

target_link_libraries(threading-local-executor PUBLIC
  TBB::tbb
  yutil
)

_ydb_sdk_install_targets(TARGETS threading-local-executor)
