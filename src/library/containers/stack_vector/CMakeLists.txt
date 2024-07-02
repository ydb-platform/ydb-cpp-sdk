add_library(containers-stack_vector INTERFACE)

target_link_libraries(containers-stack_vector INTERFACE
  yutil
)

_ydb_sdk_install_targets(TARGETS containers-stack_vector)
