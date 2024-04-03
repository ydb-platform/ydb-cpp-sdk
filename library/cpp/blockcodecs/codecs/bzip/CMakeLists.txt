add_library(blockcodecs-codecs-bzip INTERFACE)

target_link_libraries(blockcodecs-codecs-bzip INTERFACE
  yutil
  BZip2::BZip2
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-bzip.global blockcodecs-codecs-bzip)

target_link_libraries(blockcodecs-codecs-bzip.global PUBLIC
  yutil
  BZip2::BZip2
  cpp-blockcodecs-core
)
target_sources(blockcodecs-codecs-bzip.global PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/blockcodecs/codecs/bzip/bzip.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(blockcodecs-codecs-bzip INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-bzip.global PUBLIC
    
  )
endif()
