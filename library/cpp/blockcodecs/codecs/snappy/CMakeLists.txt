add_library(blockcodecs-codecs-snappy INTERFACE)

target_link_libraries(blockcodecs-codecs-snappy INTERFACE
  yutil
  Snappy::snappy
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-snappy.global blockcodecs-codecs-snappy)

target_link_libraries(blockcodecs-codecs-snappy.global PUBLIC
  yutil
  Snappy::snappy
  cpp-blockcodecs-core
)

target_sources(blockcodecs-codecs-snappy.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/snappy/snappy.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(blockcodecs-codecs-snappy INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-snappy.global PUBLIC
    
  )
endif()
