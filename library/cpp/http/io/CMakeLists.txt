add_library(cpp-http-io)

target_link_libraries(cpp-http-io PUBLIC
  yutil
  library-cpp-blockcodecs
  cpp-streams-brotli
  cpp-streams-bzip2
  cpp-streams-lzma
  cpp-string_utils-stream
  cpp-string_utils-misc
)

target_sources(cpp-http-io PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/http/io/chunk.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/io/compression.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/io/headers.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/io/stream.cpp
)


