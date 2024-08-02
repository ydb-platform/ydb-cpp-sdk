if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(library-cache)

target_link_libraries(library-cache PUBLIC
  yutil
)

target_sources(library-cache
  PRIVATE
    cache.cpp
    thread_safe_cache.cpp
)

_ydb_sdk_install_targets(TARGETS library-cache)
