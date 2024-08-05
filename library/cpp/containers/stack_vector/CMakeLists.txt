if (YDB_SDK_TESTS)
  add_ydb_test(NAME containers-stack_vector_ut
    SOURCES
      stack_vec_ut.cpp
    LINK_LIBRARIES
      containers-stack_vector
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(containers-stack_vector)

target_link_libraries(containers-stack_vector PUBLIC
  yutil
)

target_sources(containers-stack_vector PRIVATE
  stack_vec.cpp
)

_ydb_sdk_install_targets(TARGETS containers-stack_vector)
