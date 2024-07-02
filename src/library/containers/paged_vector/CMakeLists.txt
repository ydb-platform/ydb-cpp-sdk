add_library(containers-paged_vector)

target_link_libraries(containers-paged_vector PUBLIC
  yutil
)

target_sources(containers-paged_vector PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector/paged_vector.cpp
)

_ydb_sdk_install_targets(TARGETS containers-paged_vector)
