add_library(cpp-streams-bzip2)

target_link_libraries(cpp-streams-bzip2 PUBLIC
  yutil
  BZip2::BZip2
)

target_sources(cpp-streams-bzip2 PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
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
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
