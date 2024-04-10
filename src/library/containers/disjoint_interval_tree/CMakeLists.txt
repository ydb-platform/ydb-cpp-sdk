add_library(cpp-containers-disjoint_interval_tree)

target_link_libraries(cpp-containers-disjoint_interval_tree PUBLIC
  yutil
)

target_sources(cpp-containers-disjoint_interval_tree PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/containers/disjoint_interval_tree/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/containers/disjoint_interval_tree/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/containers/disjoint_interval_tree/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


