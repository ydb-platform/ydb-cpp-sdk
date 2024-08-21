if (YDB_SDK_TESTS)
  add_ydb_test(NAME string_utils-relaxed_escaper-ut
    SOURCES
      relaxed_escaper_ut.cpp
    LINK_LIBRARIES
      string_utils-relaxed_escaper
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(string_utils-relaxed_escaper)

target_link_libraries(string_utils-relaxed_escaper
  PUBLIC
    yutil
)

target_sources(string_utils-relaxed_escaper
  PRIVATE
    relaxed_escaper.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-relaxed_escaper)
