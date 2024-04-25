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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/blockcodecs/codecs/zlib/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zlib/zlib.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/zlib/zlib.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/zlib/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zlib/zlib.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/zlib/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zlib/zlib.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zlib/zlib.cpp
>>>>>>> origin/main
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-zlib INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-zlib.global PUBLIC
    
  )
endif()
