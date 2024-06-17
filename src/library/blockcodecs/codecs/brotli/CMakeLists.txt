add_library(blockcodecs-codecs-brotli INTERFACE)

target_link_libraries(blockcodecs-codecs-brotli INTERFACE
  yutil
  Brotli::enc
  Brotli::dec
  blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-brotli.global blockcodecs-codecs-brotli)

target_link_libraries(blockcodecs-codecs-brotli.global PUBLIC
  yutil
  Brotli::enc
  Brotli::dec
  blockcodecs-core
)

target_sources(blockcodecs-codecs-brotli.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/brotli/brotli.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-brotli.global.wholearchive)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-brotli)
