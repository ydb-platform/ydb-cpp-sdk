add_subdirectory(small)

add_library(library-getopt INTERFACE)
target_link_libraries(library-getopt INTERFACE
  yutil
  library-getopt-small
  library-svnversion
  library-build_info
)

add_global_library_for(library-getopt.global library-getopt)
target_link_libraries(library-getopt.global PUBLIC
  yutil
  library-getopt-small
  library-svnversion
  library-build_info
)
target_sources(library-getopt.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/getopt/print.cpp
)

_ydb_sdk_install_targets(TARGETS library-getopt)
_ydb_sdk_install_targets(TARGETS library-getopt.global.wholearchive)
