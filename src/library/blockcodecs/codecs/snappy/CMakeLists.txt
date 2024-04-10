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
<<<<<<< HEAD:src/library/blockcodecs/codecs/snappy/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/snappy/snappy.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/snappy/snappy.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/snappy/CMakeLists.txt
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-snappy INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-snappy.global PUBLIC
    
  )
endif()
