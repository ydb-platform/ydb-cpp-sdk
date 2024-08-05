_ydb_sdk_add_library(threading-equeue)

target_link_libraries(threading-equeue PUBLIC
  yutil
)

target_sources(threading-equeue PRIVATE
  equeue.cpp
)

_ydb_sdk_install_targets(TARGETS threading-equeue)
