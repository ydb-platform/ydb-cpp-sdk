add_library(blockcodecs-codecs-lz4 INTERFACE)

target_link_libraries(blockcodecs-codecs-lz4 INTERFACE
  contrib-libs-cxxsupp
  yutil
  contrib-libs-lz4
  cpp-blockcodecs-core
  cpp-string_builder
)

add_global_library_for(blockcodecs-codecs-lz4.global blockcodecs-codecs-lz4)

target_link_libraries(blockcodecs-codecs-lz4.global PUBLIC
  contrib-libs-cxxsupp
  yutil
  contrib-libs-lz4
  cpp-blockcodecs-core
  cpp-string_builder
)

target_sources(blockcodecs-codecs-lz4.global PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/blockcodecs/codecs/lz4/lz4.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(blockcodecs-codecs-lz4 INTERFACE
    contrib-libs-linux-headers
  )
  target_link_libraries(blockcodecs-codecs-lz4.global PUBLIC
    contrib-libs-linux-headers
  )
endif()
