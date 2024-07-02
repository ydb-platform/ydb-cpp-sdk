add_library(containers-intrusive_rb_tree)

target_link_libraries(containers-intrusive_rb_tree PUBLIC
  yutil
)

target_sources(containers-intrusive_rb_tree PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/intrusive_rb_tree/rb_tree.cpp
)

_ydb_sdk_install_targets(TARGETS containers-intrusive_rb_tree)
