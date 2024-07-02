add_library(library-diff)

target_link_libraries(library-diff PUBLIC
  yutil
  library-lcs
)

target_sources(library-diff PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/diff/diff.cpp
)

_ydb_sdk_install_targets(TARGETS library-diff)
