_ydb_sdk_add_library(threading-future)

target_link_libraries(threading-future PUBLIC
  yutil
)

target_sources(threading-future 
  PRIVATE
    async_semaphore.cpp
    async.cpp
    core/future.cpp
    wait/wait.cpp
)

_ydb_sdk_install_targets(TARGETS threading-future)
