add_library(cpp-containers-disjoint_interval_tree)

target_link_libraries(cpp-containers-disjoint_interval_tree PUBLIC
  yutil
)

target_sources(cpp-containers-disjoint_interval_tree PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/containers/disjoint_interval_tree/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/containers/disjoint_interval_tree/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree/disjoint_interval_tree.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


