add_library(digest-murmur)

target_link_libraries(digest-murmur PUBLIC
  yutil
)

target_sources(digest-murmur PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/murmur/murmur.cpp
)

_ydb_sdk_install_targets(TARGETS digest-murmur)
