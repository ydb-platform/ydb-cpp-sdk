add_subdirectory(protos)

add_library(library-cpp-retry)
target_link_libraries(library-cpp-retry PUBLIC
  yutil
  cpp-retry-protos
)

target_sources(library-cpp-retry PRIVATE
<<<<<<< HEAD:src/library/retry/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/utils.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/retry/utils.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/retry/CMakeLists.txt
)