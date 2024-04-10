add_library(cpp-containers-intrusive_rb_tree)

target_link_libraries(cpp-containers-intrusive_rb_tree PUBLIC
  yutil
)

target_sources(cpp-containers-intrusive_rb_tree PRIVATE
<<<<<<< HEAD:src/library/containers/intrusive_rb_tree/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/intrusive_rb_tree/rb_tree.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/containers/intrusive_rb_tree/rb_tree.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/containers/intrusive_rb_tree/CMakeLists.txt
)


