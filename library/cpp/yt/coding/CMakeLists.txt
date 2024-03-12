add_library(cpp-yt-coding INTERFACE)

target_link_libraries(cpp-yt-coding INTERFACE
  
  yutil
  cpp-yt-exception
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-yt-coding INTERFACE
    
  )
endif()
