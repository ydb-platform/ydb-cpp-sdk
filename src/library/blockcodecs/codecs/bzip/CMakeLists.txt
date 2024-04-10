add_library(blockcodecs-codecs-bzip INTERFACE)

target_link_libraries(blockcodecs-codecs-bzip INTERFACE
  yutil
  BZip2::BZip2
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-bzip.global blockcodecs-codecs-bzip)

target_link_libraries(blockcodecs-codecs-bzip.global PUBLIC
  yutil
  BZip2::BZip2
  cpp-blockcodecs-core
)
target_sources(blockcodecs-codecs-bzip.global PRIVATE
<<<<<<< HEAD:src/library/blockcodecs/codecs/bzip/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/bzip/bzip.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/bzip/bzip.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/bzip/CMakeLists.txt
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-bzip INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-bzip.global PUBLIC
    
  )
endif()
