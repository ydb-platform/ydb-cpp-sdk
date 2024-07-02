add_library(library-svnversion)

target_link_libraries(library-svnversion PUBLIC
  yutil
  string_utils-misc
)

target_sources(library-svnversion PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svnversion.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svn_interface.c
)

_ydb_sdk_install_targets(TARGETS library-svnversion)
