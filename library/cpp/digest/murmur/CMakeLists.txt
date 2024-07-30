add_library(digest-murmur)

target_link_libraries(digest-murmur
  PUBLIC
    yutil
)

target_sources(digest-murmur
  PRIVATE
    murmur.cpp
)

_ydb_sdk_install_targets(TARGETS digest-murmur)
