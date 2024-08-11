if (YDB_SDK_TESTS)
  add_ydb_test(NAME streams-bzip2-ut
    SOURCES
      bzip2_ut.cpp
    LINK_LIBRARIES
      streams-bzip2
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(streams-bzip2)

target_link_libraries(streams-bzip2
  PUBLIC
    yutil
  PRIVATE
    BZip2::BZip2
)

target_sources(streams-bzip2 PRIVATE
  bzip2.cpp
)

_ydb_sdk_install_targets(TARGETS streams-bzip2)
