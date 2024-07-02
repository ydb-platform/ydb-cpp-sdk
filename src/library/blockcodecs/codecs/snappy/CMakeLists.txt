add_library(blockcodecs-codecs-snappy INTERFACE)

target_link_libraries(blockcodecs-codecs-snappy INTERFACE
  yutil
  Snappy::snappy
  blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-snappy.global blockcodecs-codecs-snappy)

target_link_libraries(blockcodecs-codecs-snappy.global PUBLIC
  yutil
  Snappy::snappy
  blockcodecs-core
)

target_sources(blockcodecs-codecs-snappy.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/snappy/snappy.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-snappy.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-snappy)
