add_library(cpp-streams-brotli)

target_link_libraries(cpp-streams-brotli PUBLIC
  yutil
  Brotli::enc
  Brotli::dec
)

target_sources(cpp-streams-brotli PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/streams/brotli/brotli.cpp
)
