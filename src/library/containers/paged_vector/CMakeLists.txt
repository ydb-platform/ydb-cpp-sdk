add_library(cpp-containers-paged_vector)

target_link_libraries(cpp-containers-paged_vector PUBLIC
  yutil
)

target_sources(cpp-containers-paged_vector PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/containers/paged_vector/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector/paged_vector.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/containers/paged_vector/paged_vector.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/containers/paged_vector/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector/paged_vector.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/containers/paged_vector/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector/paged_vector.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector/paged_vector.cpp
>>>>>>> origin/main
)


