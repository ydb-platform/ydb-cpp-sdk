add_library(blockcodecs-codecs-fastlz INTERFACE)

target_link_libraries(blockcodecs-codecs-fastlz INTERFACE
  yutil
  FastLZ
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-fastlz.global blockcodecs-codecs-fastlz)

target_link_libraries(blockcodecs-codecs-fastlz.global PUBLIC
  yutil
  FastLZ
  cpp-blockcodecs-core
)

target_sources(blockcodecs-codecs-fastlz.global PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/blockcodecs/codecs/fastlz/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/fastlz/fastlz.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/fastlz/fastlz.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/fastlz/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/fastlz/fastlz.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/fastlz/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-fastlz INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-fastlz.global PUBLIC
    
  )
endif()
