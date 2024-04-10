add_library(library-cpp-iterator)

target_link_libraries(library-cpp-iterator PUBLIC
  yutil
)

target_sources(library-cpp-iterator PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/cartesian_product.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/concatenate.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/enumerate.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/iterate_keys.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/iterate_values.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/filtering.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/functools.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/mapped.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/iterator/zip.cpp
)
