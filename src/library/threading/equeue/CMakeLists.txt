add_library(cpp-threading-equeue)

target_link_libraries(cpp-threading-equeue PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-equeue PRIVATE
<<<<<<< HEAD:src/library/threading/equeue/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/equeue/equeue.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/equeue/equeue.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/equeue/CMakeLists.txt
)
