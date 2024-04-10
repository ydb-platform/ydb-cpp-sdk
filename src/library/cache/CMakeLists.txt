add_library(library-cpp-cache)

target_link_libraries(library-cpp-cache PUBLIC
  yutil
)

target_sources(library-cpp-cache PRIVATE
<<<<<<< HEAD:src/library/cache/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/cache/thread_safe_cache.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cache/cache.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cache/thread_safe_cache.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cache/CMakeLists.txt
)
