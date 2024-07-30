add_library(containers-stack_vector)

target_link_libraries(containers-stack_vector PUBLIC
  yutil
)

target_sources(containers-stack_vector PUBLIC
  stack_vec.cpp
)

_ydb_sdk_install_targets(TARGETS containers-stack_vector)
