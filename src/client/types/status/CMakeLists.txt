_ydb_sdk_add_library(client-types-status)

target_link_libraries(client-types-status PUBLIC
  yutil
  threading-future
  impl-internal-plain_status
  client-types
  client-types-fatal_error_handlers
  yql-public-issue
)

target_sources(client-types-status PRIVATE
  status.cpp
)

_ydb_sdk_install_targets(TARGETS client-types-status)
