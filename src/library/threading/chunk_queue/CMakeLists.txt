add_library(cpp-threading-chunk_queue)

target_link_libraries(cpp-threading-chunk_queue PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-chunk_queue PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/threading/chunk_queue/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/chunk_queue/queue.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/chunk_queue/queue.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/chunk_queue/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/chunk_queue/queue.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/chunk_queue/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/chunk_queue/queue.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
