add_subdirectory(internal)

add_library(cpp-digest-argonish)

target_link_libraries(cpp-digest-argonish PUBLIC
  yutil
  cpp-threading-poor_man_openmp
  internal-proxies-avx2
  internal-proxies-ref
  internal-proxies-sse2
  internal-proxies-sse41
  internal-proxies-ssse3
)

target_sources(cpp-digest-argonish PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/digest/argonish/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/factory/factory.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/argonish/factory/factory.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/factory/factory.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/factory/factory.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
