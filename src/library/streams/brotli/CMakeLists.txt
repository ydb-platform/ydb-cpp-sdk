add_library(cpp-streams-brotli)

target_link_libraries(cpp-streams-brotli PUBLIC
  yutil
  Brotli::enc
  Brotli::dec
)

target_sources(cpp-streams-brotli PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
)
