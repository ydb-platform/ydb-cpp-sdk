add_library(cpp-yt-small_containers INTERFACE)

target_link_libraries(cpp-yt-small_containers INTERFACE
  yutil
  cpp-yt-assert
  cpp-yt-malloc
  cpp-yt-misc
)
