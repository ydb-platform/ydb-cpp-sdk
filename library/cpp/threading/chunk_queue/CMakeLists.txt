if (YDB_SDK_TESTS)
  add_ydb_test(NAME threading-chunk_queue-ut
    SOURCES
      queue_ut.cpp
    LINK_LIBRARIES
      threading-chunk_queue
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(threading-chunk_queue)

target_link_libraries(threading-chunk_queue PUBLIC
  yutil
)

target_sources(threading-chunk_queue PRIVATE
  queue.cpp
)

_ydb_sdk_install_targets(TARGETS threading-chunk_queue)
