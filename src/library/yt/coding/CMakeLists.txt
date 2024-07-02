add_library(library-yt-coding INTERFACE)

target_link_libraries(library-yt-coding INTERFACE
  yutil
  library-yt-exception
)

_ydb_sdk_install_targets(TARGETS library-yt-coding)
