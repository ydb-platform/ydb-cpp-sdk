add_library(cpp-threading-atomic)

target_link_libraries(cpp-threading-atomic PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-atomic PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/threading/atomic/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/atomic/bool.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/atomic/bool.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/atomic/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/atomic/bool.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/atomic/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/atomic/bool.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/atomic/bool.cpp
>>>>>>> origin/main
)
