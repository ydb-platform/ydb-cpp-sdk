add_library(monlib-exception)

target_link_libraries(monlib-exception PUBLIC
  yutil
)

target_sources(monlib-exception PRIVATE
  exception.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-exception)
