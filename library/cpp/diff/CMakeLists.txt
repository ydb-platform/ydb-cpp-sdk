if (YDB_SDK_TESTS)
  add_ydb_test(NAME diff-ut
    SOURCES
      diff_ut.cpp
    LINK_LIBRARIES
      diff
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

add_library(diff)

target_link_libraries(diff
  PUBLIC
    yutil
    lcs
)

target_sources(diff PRIVATE
  diff.cpp
)

_ydb_sdk_install_targets(TARGETS diff)
