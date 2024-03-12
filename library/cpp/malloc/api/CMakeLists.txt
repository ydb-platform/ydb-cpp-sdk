add_library(cpp-malloc-api)

target_link_libraries(cpp-malloc-api PUBLIC
)

target_sources(cpp-malloc-api PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/malloc/api/malloc.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-malloc-api PUBLIC
    
  )
endif()
