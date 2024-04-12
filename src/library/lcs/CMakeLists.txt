add_library(library-cpp-lcs)

target_link_libraries(library-cpp-lcs PUBLIC
  yutil
  cpp-containers-paged_vector
)

target_sources(library-cpp-lcs PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/lcs/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/lcs/lcs_via_lis.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/lcs/lcs_via_lis.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/lcs/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/lcs/lcs_via_lis.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
