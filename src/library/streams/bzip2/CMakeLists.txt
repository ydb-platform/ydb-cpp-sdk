add_library(streams-bzip2)

target_link_libraries(streams-bzip2 PUBLIC
  yutil
  BZip2::BZip2
)

target_sources(streams-bzip2 PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/bzip2/bzip2.cpp
)

_ydb_sdk_install_targets(TARGETS streams-bzip2)
