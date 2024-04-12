add_subdirectory(json)
add_subdirectory(node)

add_library(library-cpp-yson)

target_link_libraries(library-cpp-yson PUBLIC
  yutil
  cpp-yt-misc
  cpp-yt-yson
)

target_sources(library-cpp-yson PRIVATE
<<<<<<< HEAD
  consumer.cpp
  lexer.cpp
  parser.cpp
  token.cpp
  tokenizer.cpp
  varint.cpp
  writer.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/consumer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/lexer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/parser.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/token.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/tokenizer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/varint.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/writer.cpp
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


