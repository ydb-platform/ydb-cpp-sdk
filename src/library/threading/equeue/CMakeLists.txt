add_library(threading-equeue)

target_link_libraries(threading-equeue PUBLIC
  yutil
  deprecated-atomic
)

target_sources(threading-equeue PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/equeue/equeue.cpp
)

_ydb_sdk_install_targets(TARGETS threading-equeue)
