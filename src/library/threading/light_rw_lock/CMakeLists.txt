add_library(threading-light_rw_lock)

target_link_libraries(threading-light_rw_lock PUBLIC
  yutil
)

target_sources(threading-light_rw_lock PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/light_rw_lock/lightrwlock.cpp
)

_ydb_sdk_install_targets(TARGETS threading-light_rw_lock)
