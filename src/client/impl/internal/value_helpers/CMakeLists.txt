_ydb_sdk_add_library(impl-internal-value_helpers)

target_link_libraries(impl-internal-value_helpers PUBLIC
  yutil
  api-protos
  client-types-fatal_error_handlers
)

target_sources(impl-internal-value_helpers PRIVATE
  helpers.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-value_helpers)
