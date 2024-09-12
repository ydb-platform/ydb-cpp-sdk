if (YDB_SDK_TESTS)
  add_ydb_test(NAME threading-future-async_ut
    SOURCES
      async_ut.cpp
    LINK_LIBRARIES
      threading-future
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME threading-future-legacy_future_ut
    SOURCES
      legacy_future_ut.cpp
    LINK_LIBRARIES
      threading-future
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME threading-future-async_semaphore_ut
    SOURCES
      async_semaphore_ut.cpp
    LINK_LIBRARIES
      threading-future
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME threading-future-future_ut
    SOURCES
      future_ut.cpp
    LINK_LIBRARIES
      threading-future
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME threading-future-future_mt_ut
    SOURCES
      future_mt_ut.cpp
    LINK_LIBRARIES
      threading-future
      cpp-testing-unittest_main
    LABELS
      unit-multithreaded
  )
  set_yunittest_property(
    TEST
      threading-future-future_mt_ut
    PROPERTY
      TIMEOUT
      1500
  )
endif(YDB_SDK_TESTS)

_ydb_sdk_add_library(threading-future)

target_link_libraries(threading-future PUBLIC
  yutil
)

target_sources(threading-future
  PRIVATE
    async_semaphore.cpp
    async.cpp
    core/future.cpp
    wait/wait.cpp
)

_ydb_sdk_install_targets(TARGETS threading-future)
