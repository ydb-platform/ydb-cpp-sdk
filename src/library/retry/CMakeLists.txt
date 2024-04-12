add_subdirectory(protos)

add_library(library-cpp-retry)
target_link_libraries(library-cpp-retry PUBLIC
  yutil
  cpp-retry-protos
)

target_sources(library-cpp-retry PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/utils.cpp
)