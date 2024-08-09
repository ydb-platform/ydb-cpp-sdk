if (YDB_SDK_TESTS)
  add_ydb_test(NAME lcs-lcs_via_lis_ut
    SOURCES
      lcs_via_lis_ut.cpp
    LINK_LIBRARIES
      lcs
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(lcs)

target_link_libraries(lcs
  PUBLIC
    yutil
    containers-paged_vector
)

target_sources(lcs PRIVATE
  lcs_via_lis.cpp
)

_ydb_sdk_install_targets(TARGETS lcs)
