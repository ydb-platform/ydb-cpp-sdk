add_library(threading-poor_man_openmp)

target_link_libraries(threading-poor_man_openmp PUBLIC
  yutil
  deprecated-atomic
)

target_sources(threading-poor_man_openmp PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/poor_man_openmp/thread_helper.cpp
)

_ydb_sdk_install_targets(TARGETS threading-poor_man_openmp)
