_ydb_sdk_add_library(client-types-fatal_error_handlers)

target_link_libraries(client-types-fatal_error_handlers PUBLIC
  yutil
  client-types-exceptions
)

target_sources(client-types-fatal_error_handlers PRIVATE
  handlers.cpp
)

_ydb_sdk_install_targets(TARGETS client-types-fatal_error_handlers)
