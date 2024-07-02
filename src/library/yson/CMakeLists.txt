add_subdirectory(json)
add_subdirectory(node)

add_library(library-yson)

target_link_libraries(library-yson PUBLIC
  yutil
  library-yt-misc
  library-yt-yson
)

target_sources(library-yson PRIVATE
  consumer.cpp
  lexer.cpp
  parser.cpp
  token.cpp
  tokenizer.cpp
  varint.cpp
  writer.cpp
)

_ydb_sdk_install_targets(TARGETS library-yson)
