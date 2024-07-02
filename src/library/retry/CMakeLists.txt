add_subdirectory(protos)

add_library(library-retry)
target_link_libraries(library-retry PUBLIC
  yutil
  library-retry-protos
)

target_sources(library-retry PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/retry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/retry/utils.cpp
)

_ydb_sdk_install_targets(TARGETS library-retry)
