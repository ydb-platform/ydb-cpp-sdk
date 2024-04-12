add_library(cpp-threading-equeue)

target_link_libraries(cpp-threading-equeue PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-equeue PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/threading/equeue/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/equeue/equeue.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/equeue/equeue.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/equeue/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/equeue/equeue.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/equeue/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/equeue/equeue.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
