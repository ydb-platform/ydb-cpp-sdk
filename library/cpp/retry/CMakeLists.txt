add_subdirectory(protos)

add_library(library-cpp-retry)
target_link_libraries(library-cpp-retry PUBLIC
  yutil
  cpp-retry-protos
)

target_sources(library-cpp-retry PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/retry/retry.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/retry/utils.cpp
)