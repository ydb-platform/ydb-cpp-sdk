add_library(resource)

target_link_libraries(resource
  PUBLIC
    yutil
    blockcodecs-core
    blockcodecs-codecs-zstd
)

target_sources(resource
  PRIVATE
    registry.cpp
    resource.cpp
)

_ydb_sdk_install_targets(TARGETS resource)
