add_library(library-cpp-terminate_handler)
target_link_libraries(library-cpp-terminate_handler PUBLIC
  yutil
)
target_sources(library-cpp-terminate_handler PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/terminate_handler/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/terminate_handler/segv_handler.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/terminate_handler/segv_handler.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/terminate_handler/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/terminate_handler/segv_handler.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/terminate_handler/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_global_library_for(library-cpp-terminate_handler.global library-cpp-terminate_handler)
target_link_libraries(library-cpp-terminate_handler.global PUBLIC
  yutil
)
target_sources(library-cpp-terminate_handler.global PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/terminate_handler/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/terminate_handler/terminate_handler.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/terminate_handler/terminate_handler.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/terminate_handler/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/terminate_handler/terminate_handler.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/terminate_handler/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
