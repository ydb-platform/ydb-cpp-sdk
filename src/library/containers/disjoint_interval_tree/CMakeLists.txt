add_library(containers-disjoint_interval_tree)

target_link_libraries(containers-disjoint_interval_tree PUBLIC
  yutil
)

target_sources(containers-disjoint_interval_tree PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
)

_ydb_sdk_install_targets(TARGETS containers-disjoint_interval_tree)
