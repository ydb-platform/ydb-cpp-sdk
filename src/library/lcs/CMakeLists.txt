add_library(library-cpp-lcs)

target_link_libraries(library-cpp-lcs PUBLIC
  yutil
  cpp-containers-paged_vector
)

target_sources(library-cpp-lcs PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/lcs/lcs_via_lis.cpp
)
