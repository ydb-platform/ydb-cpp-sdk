add_library(blockcodecs-codecs-zstd INTERFACE)

target_link_libraries(blockcodecs-codecs-zstd INTERFACE
  yutil
  ZSTD::ZSTD
  blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-zstd.global blockcodecs-codecs-zstd)

target_link_libraries(blockcodecs-codecs-zstd.global PUBLIC
  yutil
  ZSTD::ZSTD
  blockcodecs-core
)

target_sources(blockcodecs-codecs-zstd.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zstd/zstd.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-zstd.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-zstd)
