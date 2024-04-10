add_library(library-cpp-diff)

target_link_libraries(library-cpp-diff PUBLIC
  yutil
  library-cpp-lcs
)

target_sources(library-cpp-diff PRIVATE
<<<<<<< HEAD:src/library/diff/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/diff/diff.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/diff/diff.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/diff/CMakeLists.txt
)
