add_library(library-cpp-cache)

target_link_libraries(library-cpp-cache PUBLIC
  yutil
)

target_sources(library-cpp-cache PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/cache/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/thread_safe_cache.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cache/thread_safe_cache.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cache/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/thread_safe_cache.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cache/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/thread_safe_cache.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/thread_safe_cache.cpp
>>>>>>> origin/main
)
