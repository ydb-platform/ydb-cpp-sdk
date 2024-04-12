add_library(cpp-streams-brotli)

target_link_libraries(cpp-streams-brotli PUBLIC
  yutil
  Brotli::enc
  Brotli::dec
)

target_sources(cpp-streams-brotli PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/streams/brotli/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/brotli/brotli.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/brotli/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
