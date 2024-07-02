add_library(library-lcs)

target_link_libraries(library-lcs PUBLIC
  yutil
  containers-paged_vector
)

target_sources(library-lcs PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/lcs/lcs_via_lis.cpp
)

_ydb_sdk_install_targets(TARGETS library-lcs)
