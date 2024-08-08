if (YDB_SDK_TESTS)
  add_ydb_test(NAME http-io-chunk_ut
    SOURCES
      chunk_ut.cpp
    LINK_LIBRARIES
      http-io
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME http-io-compression_ut
    SOURCES
      compression_ut.cpp
    LINK_LIBRARIES
      http-io
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME http-io-headers_ut
    SOURCES
      headers_ut.cpp
    LINK_LIBRARIES
      http-io
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME http-io-stream_ut
    SOURCES
      stream_ut.cpp
    LINK_LIBRARIES
      yutil
      http-io
      http-server
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME http-io-stream_ut_medium
    SOURCES
      stream_ut_medium.cpp
    LINK_LIBRARIES
      http-io
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif(YDB_SDK_TESTS)

_ydb_sdk_add_library(http-io)

target_link_libraries(http-io
  PUBLIC
    yutil
  PRIVATE
    blockcodecs
    streams-brotli
    streams-bzip2
    streams-lzma
)

target_sources(http-io
  PRIVATE
    chunk.cpp
    compression.cpp
    headers.cpp
    stream.cpp
)

_ydb_sdk_install_targets(TARGETS http-io)
