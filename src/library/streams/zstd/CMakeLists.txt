add_library(cpp-streams-zstd)
target_link_libraries(cpp-streams-zstd PUBLIC
  yutil
  ZSTD::ZSTD
)
target_sources(cpp-streams-zstd PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/streams/zstd/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/zstd/zstd.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/zstd/zstd.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/zstd/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/zstd/zstd.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/zstd/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/zstd/zstd.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
