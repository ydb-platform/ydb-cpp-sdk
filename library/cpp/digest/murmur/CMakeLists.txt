if (YDB_SDK_TESTS)
  add_ydb_test(NAME digest-murmur_ut
    SOURCES
      murmur_ut.cpp
    LINK_LIBRARIES
      digest-murmur
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

add_library(digest-murmur)

target_link_libraries(digest-murmur
  PUBLIC
    yutil
)

target_sources(digest-murmur
  PRIVATE
    murmur.cpp
)

_ydb_sdk_install_targets(TARGETS digest-murmur)
