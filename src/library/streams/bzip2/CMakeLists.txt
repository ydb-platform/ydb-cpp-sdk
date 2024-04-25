add_library(cpp-streams-bzip2)

target_link_libraries(cpp-streams-bzip2 PUBLIC
  yutil
  BZip2::BZip2
)

target_sources(cpp-streams-bzip2 PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/streams/bzip2/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/bzip2/bzip2.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/bzip2/bzip2.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/bzip2/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/bzip2/bzip2.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/bzip2/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/bzip2/bzip2.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/bzip2/bzip2.cpp
>>>>>>> origin/main
)
