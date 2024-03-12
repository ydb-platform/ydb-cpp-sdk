add_library(cpp-containers-disjoint_interval_tree)

target_link_libraries(cpp-containers-disjoint_interval_tree PUBLIC
  yutil
)

target_sources(cpp-containers-disjoint_interval_tree PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-containers-disjoint_interval_tree PUBLIC
    
  )
endif()
