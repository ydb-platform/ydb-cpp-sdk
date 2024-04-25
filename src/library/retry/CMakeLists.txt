add_subdirectory(protos)

add_library(library-cpp-retry)
target_link_libraries(library-cpp-retry PUBLIC
  yutil
  cpp-retry-protos
)

target_sources(library-cpp-retry PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/retry/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/utils.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/retry/utils.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/retry/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/utils.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/retry/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/utils.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/utils.cpp
>>>>>>> origin/main
)