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
<<<<<<< HEAD:src/library/blockcodecs/codecs/lz4/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/lz4/lz4.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/lz4/lz4.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/lz4/CMakeLists.txt
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-lz4 INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-lz4.global PUBLIC
    
  )
endif()
