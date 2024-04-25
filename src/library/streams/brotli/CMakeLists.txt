add_library(cpp-streams-brotli)

target_link_libraries(cpp-streams-brotli PUBLIC
  yutil
  Brotli::enc
  Brotli::dec
)

target_sources(cpp-streams-brotli PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/streams/brotli/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/brotli/brotli.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/brotli/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/brotli/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/brotli/brotli.cpp
>>>>>>> origin/main
)
