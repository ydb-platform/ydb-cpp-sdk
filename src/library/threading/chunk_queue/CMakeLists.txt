add_library(cpp-threading-chunk_queue)

target_link_libraries(cpp-threading-chunk_queue PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-chunk_queue PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/threading/chunk_queue/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/chunk_queue/queue.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/chunk_queue/queue.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/chunk_queue/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/chunk_queue/queue.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
