add_library(svnversion)

target_link_libraries(svnversion PUBLIC
  yutil
)

target_sources(svnversion
  PRIVATE
    svnversion.cpp
    svn_interface.c
)

_ydb_sdk_install_targets(TARGETS svnversion)
