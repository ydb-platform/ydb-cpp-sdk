_ydb_sdk_add_library(impl-internal-logger)

target_link_libraries(impl-internal-logger PUBLIC
  yutil
  logger
)

target_sources(impl-internal-logger PRIVATE
  log.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-logger)
