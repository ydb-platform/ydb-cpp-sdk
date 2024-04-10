add_library(cpp-digest-murmur)

target_link_libraries(cpp-digest-murmur PUBLIC
  yutil
)

target_sources(cpp-digest-murmur PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/digest/murmur/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/murmur/murmur.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/murmur/murmur.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/murmur/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/murmur/murmur.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/murmur/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
