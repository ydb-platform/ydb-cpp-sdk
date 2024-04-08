add_library(cpp-streams-bzip2)

target_link_libraries(cpp-streams-bzip2 PUBLIC
  yutil
  BZip2::BZip2
)

target_sources(cpp-streams-bzip2 PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/bzip2/bzip2.cpp
)
