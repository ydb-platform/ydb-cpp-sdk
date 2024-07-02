add_library(library-http-io)

target_link_libraries(library-http-io PUBLIC
  yutil
  blockcodecs
  streams-brotli
  streams-bzip2
  streams-lzma
  string_utils-stream
  string_utils-misc
)

target_sources(library-http-io PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/chunk.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/compression.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/headers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/stream.cpp
)

_ydb_sdk_install_targets(TARGETS library-http-io)
