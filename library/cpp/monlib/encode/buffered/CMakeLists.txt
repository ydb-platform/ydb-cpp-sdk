add_library(monlib-encode-buffered)

target_link_libraries(monlib-encode-buffered PUBLIC
  yutil
  cpp-monlib-encode
  cpp-monlib-metrics
)

target_sources(monlib-encode-buffered PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/buffered/buffered_encoder_base.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/buffered/string_pool.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(monlib-encode-buffered PUBLIC
    
  )
endif()
