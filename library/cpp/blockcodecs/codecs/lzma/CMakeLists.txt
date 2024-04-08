add_library(blockcodecs-codecs-lzma INTERFACE)

target_link_libraries(blockcodecs-codecs-lzma INTERFACE
  yutil
  contrib-libs-lzmasdk
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-lzma.global blockcodecs-codecs-lzma)

target_link_libraries(blockcodecs-codecs-lzma.global PUBLIC
  yutil
  contrib-libs-lzmasdk
  cpp-blockcodecs-core
)

target_sources(blockcodecs-codecs-lzma.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/lzma/lzma.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(blockcodecs-codecs-lzma INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-lzma.global PUBLIC
    
  )
endif()
