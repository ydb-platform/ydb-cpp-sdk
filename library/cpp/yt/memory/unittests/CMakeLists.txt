add_ydb_test(NAME yt-memory-atomic_intrusive_ptr_ut GTEST
  SOURCES
    atomic_intrusive_ptr_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
    GTest::gmock
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-chunked_memory_pool_allocator_ut GTEST
  SOURCES
    chunked_memory_pool_allocator_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-chunked_memory_pool_output_ut GTEST
  SOURCES
    chunked_memory_pool_output_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-chunked_memory_pool_ut GTEST
  SOURCES
    chunked_memory_pool_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)
#[=[
TODO: missing the library `library/cpp/int128`
add_ydb_test(NAME yt-memory-erased_storage_ut GTEST
  SOURCES
    erased_storage_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)
#]=]
add_ydb_test(NAME yt-memory-free_list_ut GTEST
  SOURCES
    free_list_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-function_view_ut GTEST
  SOURCES
    function_view_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-intrusive_ptr_ut GTEST
  SOURCES
    intrusive_ptr_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
    GTest::gmock
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-ref_ut GTEST
  SOURCES
    ref_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-safe_memory_reader_ut GTEST
  SOURCES
    safe_memory_reader_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-shared_range_ut GTEST
  SOURCES
    shared_range_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-memory-weak_ptr_ut GTEST
  SOURCES
    weak_ptr_ut.cpp
  LINK_LIBRARIES
    yt-memory
    GTest::gtest_main
    GTest::gmock
  LABELS
    unit
)
