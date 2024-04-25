add_library(cpp-http-io)

target_link_libraries(cpp-http-io PUBLIC
  yutil
  library-cpp-blockcodecs
  cpp-streams-brotli
  cpp-streams-bzip2
  cpp-streams-lzma
  cpp-string_utils-stream
  cpp-string_utils-misc
)

target_sources(cpp-http-io PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD:src/library/http/io/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
<<<<<<< HEAD:src/library/http/io/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<< HEAD:src/library/http/io/CMakeLists.txt
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/chunk.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/compression.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/headers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/io/stream.cpp
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/io/chunk.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/io/compression.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/io/headers.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/io/stream.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/http/io/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/http/io/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
)


