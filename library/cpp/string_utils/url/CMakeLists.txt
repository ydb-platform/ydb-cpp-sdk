if (YDB_SDK_TESTS)
  add_ydb_test(NAME string_utils-url-ut
    SOURCES
      url_ut.cpp
    LINK_LIBRARIES
      string_utils-url
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(string_utils-url)

target_link_libraries(string_utils-url
  PUBLIC
    yutil
)

target_sources(string_utils-url PRIVATE
  url.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-url)
