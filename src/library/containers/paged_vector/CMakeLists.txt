add_library(library-containers-paged_vector)

target_link_libraries(library-containers-paged_vector PUBLIC
  yutil
)

target_sources(library-containers-paged_vector PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector/paged_vector.cpp
)
