if (YDB_SDK_TESTS)
  add_ydb_test(NAME digest-lower_case-ut
    SOURCES
      hash_ops_ut.cpp
      lchash_ut.cpp
    LINK_LIBRARIES
      digest-lower_case
      cpp-testing-unittest_main
    LABELS
      unit
  )

  target_compile_options(digest-lower_case-ut PRIVATE
    -Wno-deprecated
  )
endif()

_ydb_sdk_add_library(digest-lower_case)

target_link_libraries(digest-lower_case PUBLIC
  yutil
)

target_sources(digest-lower_case 
  PRIVATE
    lciter.cpp
    lchash.cpp
    hash_ops.cpp
)

target_compile_options(digest-lower_case PRIVATE
  -Wno-deprecated
)

_ydb_sdk_install_targets(TARGETS digest-lower_case)
