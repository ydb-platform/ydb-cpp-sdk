add_library(monlib-dynamic_counters-percentile INTERFACE)

target_link_libraries(monlib-dynamic_counters-percentile INTERFACE
  yutil
  cpp-containers-stack_vector
  cpp-monlib-dynamic_counters
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(monlib-dynamic_counters-percentile INTERFACE
    
  )
endif()
