add_library(blockcodecs-codecs-bzip INTERFACE)

target_link_libraries(blockcodecs-codecs-bzip INTERFACE
  yutil
  BZip2::BZip2
  blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-bzip.global blockcodecs-codecs-bzip)

target_link_libraries(blockcodecs-codecs-bzip.global PUBLIC
  yutil
  BZip2::BZip2
  blockcodecs-core
)
target_sources(blockcodecs-codecs-bzip.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/bzip/bzip.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-bzip.global.wholearchive)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-bzip)
