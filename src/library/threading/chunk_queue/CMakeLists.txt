add_library(threading-chunk_queue)

target_link_libraries(threading-chunk_queue PUBLIC
  yutil
  deprecated-atomic
)

target_sources(threading-chunk_queue PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/chunk_queue/queue.cpp
)

_ydb_sdk_install_targets(TARGETS threading-chunk_queue)
