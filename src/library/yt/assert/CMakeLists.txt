add_library(library-yt-assert)

target_compile_options(library-yt-assert PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(library-yt-assert PUBLIC
  yutil
)

target_sources(library-yt-assert PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/assert/assert.cpp
)

_ydb_sdk_install_targets(TARGETS library-yt-assert)
