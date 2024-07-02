add_library(blockcodecs-codecs-lzma INTERFACE)

target_link_libraries(blockcodecs-codecs-lzma INTERFACE
  yutil
  contrib-libs-lzmasdk
  blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-lzma.global blockcodecs-codecs-lzma)

target_link_libraries(blockcodecs-codecs-lzma.global PUBLIC
  yutil
  contrib-libs-lzmasdk
  blockcodecs-core
)

target_sources(blockcodecs-codecs-lzma.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/lzma/lzma.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-lzma.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-lzma)
