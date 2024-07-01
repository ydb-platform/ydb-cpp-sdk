add_library(library-containers-disjoint_interval_tree)

target_link_libraries(library-containers-disjoint_interval_tree PUBLIC
  yutil
)

target_sources(library-containers-disjoint_interval_tree PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
)
