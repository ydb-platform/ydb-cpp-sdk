add_library(digest-lower_case)

target_link_libraries(digest-lower_case PUBLIC
  yutil
)

target_sources(digest-lower_case PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case/lciter.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case/lchash.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case/hash_ops.cpp
)

_ydb_sdk_install_targets(TARGETS digest-lower_case)
