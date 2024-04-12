add_library(blockcodecs-codecs-lz4 INTERFACE)

target_link_libraries(blockcodecs-codecs-lz4 INTERFACE
  yutil
  LZ4::LZ4
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-lz4.global blockcodecs-codecs-lz4)

target_link_libraries(blockcodecs-codecs-lz4.global PUBLIC
  yutil
  LZ4::LZ4
  cpp-blockcodecs-core
)

target_sources(blockcodecs-codecs-lz4.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/lz4/lz4.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-lz4 INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-lz4.global PUBLIC
    
  )
endif()
