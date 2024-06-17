add_library(streams-zstd)
target_link_libraries(streams-zstd PUBLIC
  yutil
  ZSTD::ZSTD
)
target_sources(streams-zstd PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/zstd/zstd.cpp
)

_ydb_sdk_install_targets(TARGETS streams-zstd)
