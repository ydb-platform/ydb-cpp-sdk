if (YDB_SDK_TESTS)
  add_ydb_test(NAME streams-brotli-ut
    SOURCES
      brotli_ut.cpp
    LINK_LIBRARIES
      streams-brotli
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(streams-brotli)

target_link_libraries(streams-brotli
  PUBLIC
    yutil
  PRIVATE
    Brotli::enc
    Brotli::dec
)

target_sources(streams-brotli PRIVATE
  brotli.cpp
)

_ydb_sdk_install_targets(TARGETS streams-brotli)
