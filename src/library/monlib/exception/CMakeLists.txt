add_library(cpp-monlib-exception)

target_link_libraries(cpp-monlib-exception PUBLIC
  yutil
)

target_sources(cpp-monlib-exception PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/monlib/exception/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/exception/exception.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/exception/exception.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/exception/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/exception/exception.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/exception/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/exception/exception.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/exception/exception.cpp
>>>>>>> origin/main
)


