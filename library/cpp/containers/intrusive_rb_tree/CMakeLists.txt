add_library(cpp-containers-intrusive_rb_tree)

target_link_libraries(cpp-containers-intrusive_rb_tree PUBLIC
  yutil
)

target_sources(cpp-containers-intrusive_rb_tree PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/containers/intrusive_rb_tree/rb_tree.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-containers-intrusive_rb_tree PUBLIC
    
  )
endif()
