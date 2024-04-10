add_subdirectory(codecs)
add_subdirectory(core)

add_library(library-cpp-blockcodecs)

target_link_libraries(library-cpp-blockcodecs PUBLIC
  yutil
  cpp-blockcodecs-core
  blockcodecs-codecs-brotli
  blockcodecs-codecs-bzip
  blockcodecs-codecs-fastlz
  blockcodecs-codecs-lz4
  blockcodecs-codecs-lzma
  blockcodecs-codecs-snappy
  blockcodecs-codecs-zlib
  blockcodecs-codecs-zstd
)

target_sources(library-cpp-blockcodecs PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/stream.cpp
)
