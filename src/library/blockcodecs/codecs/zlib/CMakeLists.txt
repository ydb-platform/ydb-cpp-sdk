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
<<<<<<< HEAD:src/library/blockcodecs/codecs/zlib/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zlib/zlib.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/zlib/zlib.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/zlib/CMakeLists.txt
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-zlib INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-zlib.global PUBLIC
    
  )
endif()
