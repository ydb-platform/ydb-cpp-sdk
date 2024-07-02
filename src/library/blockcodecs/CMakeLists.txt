add_subdirectory(codecs)
add_subdirectory(core)

add_library(blockcodecs)

target_link_libraries(blockcodecs PUBLIC
  yutil
  blockcodecs-core
  blockcodecs-codecs-brotli
  blockcodecs-codecs-bzip
  blockcodecs-codecs-fastlz
  blockcodecs-codecs-lz4
  blockcodecs-codecs-lzma
  blockcodecs-codecs-snappy
  blockcodecs-codecs-zlib
  blockcodecs-codecs-zstd
)

target_sources(blockcodecs PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/stream.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs)
