_ydb_sdk_add_library(impl-internal-scheme_helpers)

target_link_libraries(impl-internal-scheme_helpers PUBLIC
  api-protos
  client-ydb_driver
)

target_sources(impl-internal-scheme_helpers PRIVATE
  helpers.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-scheme_helpers)
