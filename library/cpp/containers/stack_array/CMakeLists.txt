add_library(cpp-containers-stack_array)

target_link_libraries(cpp-containers-stack_array PUBLIC
  yutil
)

target_sources(cpp-containers-stack_array PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/containers/stack_array/range_ops.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/containers/stack_array/stack_array.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-containers-stack_array PUBLIC
    
  )
endif()
