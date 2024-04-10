add_library(blockcodecs-codecs-zstd INTERFACE)

target_link_libraries(blockcodecs-codecs-zstd INTERFACE
  yutil
  ZSTD::ZSTD
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-zstd.global blockcodecs-codecs-zstd)

target_link_libraries(blockcodecs-codecs-zstd.global PUBLIC
  yutil
  ZSTD::ZSTD
  cpp-blockcodecs-core
)

target_sources(blockcodecs-codecs-zstd.global PRIVATE
<<<<<<< HEAD:src/library/blockcodecs/codecs/zstd/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zstd/zstd.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/zstd/zstd.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/zstd/CMakeLists.txt
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-zstd INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-zstd.global PUBLIC
    
  )
endif()
