add_library(blockcodecs-codecs-zlib INTERFACE)

target_link_libraries(blockcodecs-codecs-zlib INTERFACE
  contrib-libs-cxxsupp
  yutil
  contrib-libs-zlib
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-zlib.global blockcodecs-codecs-zlib)

target_link_libraries(blockcodecs-codecs-zlib.global PUBLIC
  contrib-libs-cxxsupp
  yutil
  contrib-libs-zlib
  cpp-blockcodecs-core

)

target_sources(blockcodecs-codecs-zlib.global PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/blockcodecs/codecs/zlib/zlib.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(blockcodecs-codecs-zlib INTERFACE
    contrib-libs-linux-headers
  )
  target_link_libraries(blockcodecs-codecs-zlib.global PUBLIC
    contrib-libs-linux-headers
  )
endif()
