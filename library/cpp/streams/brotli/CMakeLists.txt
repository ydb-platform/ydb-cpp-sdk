add_library(streams-brotli)

target_link_libraries(streams-brotli
  PUBLIC
    yutil
  PRIVATE
    Brotli::enc
    Brotli::dec
)

target_sources(streams-brotli PRIVATE
  brotli.cpp
)

_ydb_sdk_install_targets(TARGETS streams-brotli)
