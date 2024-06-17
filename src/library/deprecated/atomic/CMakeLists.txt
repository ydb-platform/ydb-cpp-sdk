add_library(deprecated-atomic INTERFACE)

target_link_libraries(deprecated-atomic INTERFACE
  yutil
)

_ydb_sdk_install_targets(TARGETS deprecated-atomic)
