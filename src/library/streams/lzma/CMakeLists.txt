add_library(cpp-streams-lzma)

target_link_libraries(cpp-streams-lzma PUBLIC
  yutil
  contrib-libs-lzmasdk
)

target_sources(cpp-streams-lzma PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/streams/lzma/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/lzma/lzma.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/lzma/lzma.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/lzma/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/lzma/lzma.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/lzma/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/lzma/lzma.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/lzma/lzma.cpp
>>>>>>> origin/main
)
