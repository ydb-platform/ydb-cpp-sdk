add_library(blockcodecs-codecs-brotli INTERFACE)

target_link_libraries(blockcodecs-codecs-brotli INTERFACE
  yutil
  Brotli::enc
  Brotli::dec
  cpp-blockcodecs-core
)

add_global_library_for(blockcodecs-codecs-brotli.global blockcodecs-codecs-brotli)

target_link_libraries(blockcodecs-codecs-brotli.global PUBLIC
  yutil
  Brotli::enc
  Brotli::dec
  cpp-blockcodecs-core
)

target_sources(blockcodecs-codecs-brotli.global PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/blockcodecs/codecs/brotli/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/brotli/brotli.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs/brotli/brotli.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/brotli/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/brotli/brotli.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/codecs/brotli/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/brotli/brotli.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs/brotli/brotli.cpp
>>>>>>> origin/main
)
