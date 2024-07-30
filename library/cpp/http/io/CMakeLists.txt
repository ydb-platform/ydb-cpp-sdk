add_library(http-io)

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
