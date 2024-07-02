add_library(streams-lzma)

target_link_libraries(streams-lzma PUBLIC
  yutil
  contrib-libs-lzmasdk
)

target_sources(streams-lzma PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/lzma/lzma.cpp
)

_ydb_sdk_install_targets(TARGETS streams-lzma)
