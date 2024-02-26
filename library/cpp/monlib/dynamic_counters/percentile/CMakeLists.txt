add_library(monlib-dynamic_counters-percentile INTERFACE)

target_link_libraries(monlib-dynamic_counters-percentile INTERFACE
  contrib-libs-cxxsupp
  yutil
  cpp-containers-stack_vector
  cpp-monlib-dynamic_counters
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(monlib-dynamic_counters-percentile INTERFACE
    contrib-libs-linux-headers
  )
endif()
