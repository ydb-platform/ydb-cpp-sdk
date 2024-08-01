add_ydb_test(NAME coroutine-engine-stack-ut GTEST
  SOURCES
    ut/stack_allocator_ut.cpp
    ut/stack_guards_ut.cpp
    ut/stack_pool_ut.cpp
    ut/stack_ut.cpp
    ut/stack_utils_ut.cpp
  LINK_LIBRARIES
    coroutine-engine
    GTest::gtest_main
  LABELS
    unit
)
