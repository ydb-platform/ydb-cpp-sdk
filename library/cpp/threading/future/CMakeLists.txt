_ydb_sdk_add_library(threading-future)

target_link_libraries(threading-future PUBLIC
  yutil
)

target_sources(threading-future 
  PRIVATE
    async_semaphore.cpp
    async.cpp
    core/future.cpp
    core/fwd.cpp
    fwd.cpp
    wait/fwd.cpp
    wait/wait.cpp
    wait/wait_group.cpp
    wait/wait_policy.cpp
)

_ydb_sdk_install_targets(TARGETS threading-future)

if (YDB_SDK_TESTS)
  add_ydb_test(NAME future-ut
    SOURCES
      future_mt_ut.cpp
      future_ut.cpp
    LINK_LIBRARIES
      yutil
      threading-future
    TEST_ARG
      --filter-file filter.txt
    LABELS
      unit
  )

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/filter.txt
    "-TFutureMultiThreadedTest::WaitAny\n"
    "-TFutureMultiThreadedTest::WaitExceptionOrAll"
  )

  add_ydb_test(NAME future-coroutine-ut GTEST
    SOURCES
      ut_gtest/coroutine_traits_ut.cpp
    LINK_LIBRARIES
      yutil
      threading-future
    LABELS
      unit
  )
endif()

add_subdirectory(subscription)
