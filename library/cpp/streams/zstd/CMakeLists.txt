add_library(streams-zstd)
target_link_libraries(streams-zstd
  PUBLIC
    yutil
  PRIVATE
    ZSTD::ZSTD
)
target_sources(streams-zstd PRIVATE
  zstd.cpp
)

_ydb_sdk_install_targets(TARGETS streams-zstd)
