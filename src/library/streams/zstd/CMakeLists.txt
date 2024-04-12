add_library(cpp-streams-zstd)
target_link_libraries(cpp-streams-zstd PUBLIC
  yutil
  ZSTD::ZSTD
)
target_sources(cpp-streams-zstd PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/streams/zstd/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/zstd/zstd.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/zstd/zstd.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/zstd/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/zstd/zstd.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
