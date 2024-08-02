if (YDB_SDK_TESTS)
  add_ydb_test(NAME containers-intrusive_rb_tree_ut
    SOURCES
      rb_tree_ut.cpp
    LINK_LIBRARIES
      containers-intrusive_rb_tree
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(containers-intrusive_rb_tree)

target_link_libraries(containers-intrusive_rb_tree PUBLIC
  yutil
)

target_sources(containers-intrusive_rb_tree PRIVATE
  rb_tree.cpp
)

_ydb_sdk_install_targets(TARGETS containers-intrusive_rb_tree)
