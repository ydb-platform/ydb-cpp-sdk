add_library(cpp-threading-atomic)

target_link_libraries(cpp-threading-atomic PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-atomic PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/threading/atomic/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/atomic/bool.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/atomic/bool.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/atomic/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/atomic/bool.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
