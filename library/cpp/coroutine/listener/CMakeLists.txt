_ydb_sdk_add_library(coroutine-listener)

target_link_libraries(coroutine-listener
  PUBLIC
    yutil
  PRIVATE
    coroutine-engine
)

target_sources(coroutine-listener PRIVATE
  listen.cpp
)

_ydb_sdk_install_targets(TARGETS coroutine-listener)
