add_subdirectory(json)
add_subdirectory(node)

add_library(library-cpp-yson)

target_link_libraries(library-cpp-yson PUBLIC
  yutil
  cpp-yt-misc
  cpp-yt-yson
)

target_sources(library-cpp-yson PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/consumer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/lexer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/parser.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/token.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/tokenizer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/varint.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/writer.cpp
)


