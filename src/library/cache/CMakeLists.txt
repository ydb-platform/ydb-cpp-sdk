add_library(library-cache)

target_link_libraries(library-cache PUBLIC
  yutil
)

target_sources(library-cache PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/thread_safe_cache.cpp
)

_ydb_sdk_install_targets(TARGETS library-cache)
