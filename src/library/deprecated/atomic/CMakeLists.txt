add_library(cpp-deprecated-atomic INTERFACE)

target_link_libraries(cpp-deprecated-atomic INTERFACE
  yutil
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(cpp-deprecated-atomic INTERFACE
    
  )
endif()
