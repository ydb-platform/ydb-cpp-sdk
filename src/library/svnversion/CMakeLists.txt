add_library(library-cpp-svnversion)

target_link_libraries(library-cpp-svnversion PUBLIC
  yutil
  cpp-string_utils-misc
)

target_sources(library-cpp-svnversion PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svnversion.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svn_interface.c
)


