add_library(terminate_handler)
target_link_libraries(terminate_handler PRIVATE
  yutil
)
target_sources(terminate_handler PRIVATE
  segv_handler.cpp
)

add_global_library_for(terminate_handler.global terminate_handler)
target_link_libraries(terminate_handler.global PRIVATE
  yutil
)
target_sources(terminate_handler.global PRIVATE
  terminate_handler.cpp
)
