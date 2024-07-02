add_library(library-resource)

target_link_libraries(library-resource PUBLIC
  yutil
  blockcodecs-core
  blockcodecs-codecs-zstd
  string_utils-misc
)

target_sources(library-resource PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/resource/resource.cpp
)

_ydb_sdk_install_targets(TARGETS library-resource)
