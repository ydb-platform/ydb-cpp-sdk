add_library(cpp-string_utils-misc INTERFACE)

target_link_libraries(cpp-string_utils-misc INTERFACE
  yutil
  cpp-string_utils-helpers
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_utils-misc INTERFACE
    
  )
endif()
