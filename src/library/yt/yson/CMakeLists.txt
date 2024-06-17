add_library(library-yt-yson)

target_compile_options(library-yt-yson PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(library-yt-yson PUBLIC
  yutil
  library-yt-yson_string
)

target_sources(library-yt-yson PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson/consumer.cpp
)

_ydb_sdk_install_targets(TARGETS library-yt-yson)
