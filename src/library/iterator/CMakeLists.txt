add_library(library-cpp-iterator)

target_link_libraries(library-cpp-iterator PUBLIC
  yutil
)

target_sources(library-cpp-iterator PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/cartesian_product.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/concatenate.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/enumerate.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/iterate_keys.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/iterate_values.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/filtering.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/functools.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/mapped.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/iterator/zip.cpp
)
