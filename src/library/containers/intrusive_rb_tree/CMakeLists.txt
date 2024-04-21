add_library(cpp-containers-intrusive_rb_tree)

target_link_libraries(cpp-containers-intrusive_rb_tree PUBLIC
  yutil
)

target_sources(cpp-containers-intrusive_rb_tree PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/intrusive_rb_tree/rb_tree.cpp
)


