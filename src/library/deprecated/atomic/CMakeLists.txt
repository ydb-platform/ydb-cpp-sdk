add_library(cpp-deprecated-atomic INTERFACE)

target_link_libraries(cpp-deprecated-atomic INTERFACE
  yutil
)
<<<<<<< HEAD
=======

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(cpp-deprecated-atomic INTERFACE
    
  )
endif()
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
