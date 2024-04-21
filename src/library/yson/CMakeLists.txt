add_subdirectory(json)
add_subdirectory(node)

add_library(library-cpp-yson)

target_link_libraries(library-cpp-yson PUBLIC
  yutil
  cpp-yt-misc
  cpp-yt-yson
)

target_sources(library-cpp-yson PRIVATE
  consumer.cpp
  lexer.cpp
  parser.cpp
  token.cpp
  tokenizer.cpp
  varint.cpp
  writer.cpp
)


