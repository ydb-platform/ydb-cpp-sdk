_ydb_sdk_add_library(streams-lzma)

target_link_libraries(streams-lzma
  PUBLIC
    yutil
  PRIVATE
    contrib-libs-lzmasdk
)

target_sources(streams-lzma PRIVATE
  lzma.cpp
)

_ydb_sdk_install_targets(TARGETS streams-lzma)
