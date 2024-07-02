add_library(streams-brotli)

target_link_libraries(streams-brotli PUBLIC
  yutil
  Brotli::enc
  Brotli::dec
)

target_sources(streams-brotli PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
)

_ydb_sdk_install_targets(TARGETS streams-brotli)
