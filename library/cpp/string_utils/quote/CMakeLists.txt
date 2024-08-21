if (YDB_SDK_TESTS)
  add_ydb_test(NAME string_utils-quote-ut
    SOURCES
      quote_ut.cpp
    LINK_LIBRARIES
      string_utils-quote
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(string_utils-quote)

target_link_libraries(string_utils-quote
  PUBLIC
    yutil
)

target_sources(string_utils-quote
  PRIVATE
    quote.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-quote)
