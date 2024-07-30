add_library(streams-bzip2)

target_link_libraries(streams-bzip2
  PUBLIC
    yutil
  PRIVATE
    BZip2::BZip2
)

target_sources(streams-bzip2 PRIVATE
  bzip2.cpp
)

_ydb_sdk_install_targets(TARGETS streams-bzip2)
