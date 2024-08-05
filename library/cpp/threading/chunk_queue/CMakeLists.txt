_ydb_sdk_add_library(threading-chunk_queue)

target_link_libraries(threading-chunk_queue PUBLIC
  yutil
)

target_sources(threading-chunk_queue PRIVATE
  queue.cpp
)

_ydb_sdk_install_targets(TARGETS threading-chunk_queue)
