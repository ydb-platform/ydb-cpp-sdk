add_library(digest-lower_case)

target_link_libraries(digest-lower_case PUBLIC
  yutil
)

target_sources(digest-lower_case 
  PRIVATE
    lciter.cpp
    lchash.cpp
    hash_ops.cpp
)

target_compile_options(digest-lower_case PRIVATE
  -Wno-deprecated
)

_ydb_sdk_install_targets(TARGETS digest-lower_case)
