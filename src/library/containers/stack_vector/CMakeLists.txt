<<<<<<< HEAD
add_library(cpp-containers-stack_vector INTERFACE)

target_link_libraries(cpp-containers-stack_vector INTERFACE
  yutil
)
=======
add_library(cpp-containers-stack_vector)

target_link_libraries(cpp-containers-stack_vector PUBLIC
  yutil
)

target_sources(cpp-containers-stack_vector PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/stack_vector/stack_vec.cpp
)


>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
