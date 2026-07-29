if (YDB_SDK_TESTS)
  add_ydb_test(NAME html-escape-ut
    SOURCES
      ut/escape_ut.cpp
    LINK_LIBRARIES
      html-escape
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(html-escape)

target_link_libraries(html-escape
  PUBLIC
    yutil
)

target_sources(html-escape
  PRIVATE
    escape.cpp
)

_ydb_sdk_install_targets(TARGETS html-escape)
