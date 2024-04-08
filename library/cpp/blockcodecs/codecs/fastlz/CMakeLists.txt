add_library(blockcodecs-codecs-fastlz INTERFACE)

target_link_libraries(blockcodecs-codecs-fastlz INTERFACE
  yutil
  FastLZ
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-fastlz.global blockcodecs-codecs-fastlz)

target_link_libraries(blockcodecs-codecs-fastlz.global PUBLIC
  yutil
  FastLZ
  cpp-blockcodecs-core
)

target_sources(blockcodecs-codecs-fastlz.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/fastlz/fastlz.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(blockcodecs-codecs-fastlz INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-fastlz.global PUBLIC
    
  )
endif()
