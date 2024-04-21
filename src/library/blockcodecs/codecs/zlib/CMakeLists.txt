add_library(blockcodecs-codecs-zlib INTERFACE)

target_link_libraries(blockcodecs-codecs-zlib INTERFACE
  yutil
  ZLIB::ZLIB
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-zlib.global blockcodecs-codecs-zlib)

target_link_libraries(blockcodecs-codecs-zlib.global PUBLIC
  yutil
  ZLIB::ZLIB
  cpp-blockcodecs-core

)

target_sources(blockcodecs-codecs-zlib.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zlib/zlib.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-zlib INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-zlib.global PUBLIC
    
  )
endif()
