add_ydb_test(NAME library-coroutine-engine-coroutine_ut
  SOURCES
    coroutine_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine
  LINK_LIBRARIES
    cpp-coroutine-engine
    cpp-testing-unittest_main
)

add_subdirectory(stack)
