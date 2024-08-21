if (YDB_SDK_TESTS)
  add_ydb_test(NAME streams-zstd-ut
    SOURCES
      zstd_ut.cpp
    LINK_LIBRARIES
      streams-zstd
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(streams-zstd)
target_link_libraries(streams-zstd
  PUBLIC
    yutil
  PRIVATE
    ZSTD::ZSTD
)
target_sources(streams-zstd PRIVATE
  zstd.cpp
)

_ydb_sdk_install_targets(TARGETS streams-zstd)
