add_library(library-yt-small_containers INTERFACE)

target_link_libraries(library-yt-small_containers INTERFACE
  yutil
  library-yt-assert
  library-yt-malloc
  library-yt-misc
)

_ydb_sdk_install_targets(TARGETS library-yt-small_containers)
