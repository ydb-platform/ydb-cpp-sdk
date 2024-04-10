add_library(cpp-threading-poor_man_openmp)

target_link_libraries(cpp-threading-poor_man_openmp PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-poor_man_openmp PRIVATE
<<<<<<< HEAD:src/library/threading/poor_man_openmp/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/poor_man_openmp/thread_helper.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/poor_man_openmp/thread_helper.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/poor_man_openmp/CMakeLists.txt
)
