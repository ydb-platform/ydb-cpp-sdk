if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(threading-light_rw_lock)

target_link_libraries(threading-light_rw_lock PUBLIC
  yutil
)

target_sources(threading-light_rw_lock PRIVATE
  lightrwlock.cpp
)

_ydb_sdk_install_targets(TARGETS threading-light_rw_lock)
