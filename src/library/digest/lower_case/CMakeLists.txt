add_library(cpp-digest-lower_case)

target_link_libraries(cpp-digest-lower_case PUBLIC
  yutil
)

target_sources(cpp-digest-lower_case PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case/lciter.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case/lchash.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case/hash_ops.cpp
)
