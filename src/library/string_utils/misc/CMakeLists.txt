add_library(cpp-string_utils-misc INTERFACE)

target_link_libraries(cpp-string_utils-misc INTERFACE
  yutil
  cpp-string_utils-helpers
)
<<<<<<< HEAD
=======

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(cpp-string_utils-misc INTERFACE
    
  )
endif()
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
