add_library(cpp-streams-lzma)

target_link_libraries(cpp-streams-lzma PUBLIC
  yutil
  contrib-libs-lzmasdk
)

target_sources(cpp-streams-lzma PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/lzma/lzma.cpp
)
