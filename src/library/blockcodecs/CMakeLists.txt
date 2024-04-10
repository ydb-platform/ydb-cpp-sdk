add_subdirectory(codecs)
add_subdirectory(core)

add_library(library-cpp-blockcodecs)

target_link_libraries(library-cpp-blockcodecs PUBLIC
  yutil
  cpp-blockcodecs-core
  blockcodecs-codecs-brotli
  blockcodecs-codecs-bzip
  blockcodecs-codecs-fastlz
  blockcodecs-codecs-lz4
  blockcodecs-codecs-lzma
  blockcodecs-codecs-snappy
  blockcodecs-codecs-zlib
  blockcodecs-codecs-zstd
)

target_sources(library-cpp-blockcodecs PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/blockcodecs/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/stream.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/stream.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/stream.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
