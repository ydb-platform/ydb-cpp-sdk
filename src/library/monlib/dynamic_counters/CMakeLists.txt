add_subdirectory(percentile)

add_library(monlib-dynamic_counters)

target_link_libraries(monlib-dynamic_counters PUBLIC
  yutil
  containers-stack_vector
  monlib-encode-json
  monlib-encode-spack
  monlib-encode-prometheus
  monlib-service-pages
  string_utils-quote
  threading-light_rw_lock
)

target_sources(monlib-dynamic_counters PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/counters.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/encode.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/golovan_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/page.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-dynamic_counters)
