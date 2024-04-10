add_library(library-cpp-diff)

target_link_libraries(library-cpp-diff PUBLIC
  yutil
  library-cpp-lcs
)

target_sources(library-cpp-diff PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/diff/diff.cpp
)
