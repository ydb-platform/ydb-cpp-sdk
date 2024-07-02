add_library(library-yt-exception)

target_compile_options(library-yt-exception PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(library-yt-exception PUBLIC
  yutil
)

target_sources(library-yt-exception PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/exception/exception.cpp
)

_ydb_sdk_install_targets(TARGETS library-yt-exception)
