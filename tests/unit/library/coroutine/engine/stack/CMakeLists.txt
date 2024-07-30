add_ydb_test(NAME coroutine-engine-stack-stack_allocator_ut
  SOURCES
    stack_allocator_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack
  LINK_LIBRARIES
    coroutine-engine
    cpp-testing-gtest_main
  LABELS
    unit
)

add_ydb_test(NAME coroutine-engine-stack-stack_guards_ut
  SOURCES
    stack_guards_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack
  LINK_LIBRARIES
    coroutine-engine
    cpp-testing-gtest_main
  LABELS
    unit
)

add_ydb_test(NAME coroutine-engine-stack-stack_pool_ut
  SOURCES
    stack_pool_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack
  LINK_LIBRARIES
    coroutine-engine
    cpp-testing-gtest_main
  LABELS
    unit
)

add_ydb_test(NAME coroutine-engine-stack-stack_ut
  SOURCES
    stack_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack
  LINK_LIBRARIES
    coroutine-engine
    cpp-testing-gtest_main
  LABELS
    unit
)

add_ydb_test(NAME coroutine-engine-stack-stack_utils_ut
  SOURCES
    stack_utils_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack
  LINK_LIBRARIES
    coroutine-engine
    cpp-testing-gtest_main
  LABELS
    unit
)
