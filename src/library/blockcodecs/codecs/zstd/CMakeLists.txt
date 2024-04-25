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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/blockcodecs/codecs/zstd/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zstd/zstd.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/zstd/zstd.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/zstd/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zstd/zstd.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/zstd/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zstd/zstd.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/zstd/zstd.cpp
>>>>>>> origin/main
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(blockcodecs-codecs-zstd INTERFACE
    
  )
  target_link_libraries(blockcodecs-codecs-zstd.global PUBLIC
    
  )
endif()
