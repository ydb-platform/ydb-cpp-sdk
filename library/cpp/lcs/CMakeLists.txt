add_library(lcs)

target_link_libraries(lcs
  PUBLIC
    yutil
    containers-paged_vector
)

target_sources(lcs PRIVATE
  lcs_via_lis.cpp
)

_ydb_sdk_install_targets(TARGETS lcs)
