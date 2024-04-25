add_library(library-cpp-resource)

target_link_libraries(library-cpp-resource PUBLIC
  yutil
  cpp-blockcodecs-core
  blockcodecs-codecs-zstd
  cpp-string_utils-misc
)

target_sources(library-cpp-resource PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/resource/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/resource.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/resource/resource.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/resource/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/resource.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/resource/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/resource.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/resource.cpp
>>>>>>> origin/main
)


