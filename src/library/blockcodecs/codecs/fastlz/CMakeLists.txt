add_library(blockcodecs-codecs-fastlz INTERFACE)

target_link_libraries(blockcodecs-codecs-fastlz INTERFACE
  yutil
  FastLZ
  blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-fastlz.global blockcodecs-codecs-fastlz)

target_link_libraries(blockcodecs-codecs-fastlz.global PUBLIC
  yutil
  FastLZ
  blockcodecs-core
)

target_sources(blockcodecs-codecs-fastlz.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/fastlz/fastlz.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-fastlz.global.wholearchive)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-fastlz)
