add_library(blockcodecs-core)

target_link_libraries(blockcodecs-core PUBLIC
  yutil
)

target_sources(blockcodecs-core PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/stream.cpp
)

_ydb_sdk_install_targets(TARGETS blockcodecs-core)
