add_library(library-cpp-resource)

target_link_libraries(library-cpp-resource PUBLIC
  yutil
  cpp-blockcodecs-core
  blockcodecs-codecs-zstd
  cpp-string_utils-misc
)

target_sources(library-cpp-resource PRIVATE
<<<<<<< HEAD:src/library/resource/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/resource.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/resource/resource.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/resource/CMakeLists.txt
)


