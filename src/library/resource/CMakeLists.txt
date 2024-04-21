add_library(library-cpp-resource)

target_link_libraries(library-cpp-resource PUBLIC
  yutil
  cpp-blockcodecs-core
  blockcodecs-codecs-zstd
  cpp-string_utils-misc
)

target_sources(library-cpp-resource PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/resource.cpp
)


