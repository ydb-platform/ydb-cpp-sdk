add_library(cpp-testing-hook)

target_link_libraries(cpp-testing-hook PUBLIC
  yutil
)

target_sources(cpp-testing-hook PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/testing/hook/CMakeLists.txt
  hook.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/hook/hook.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/testing/hook/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/hook/hook.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/testing/hook/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/hook/hook.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  hook.cpp
>>>>>>> origin/main
)
