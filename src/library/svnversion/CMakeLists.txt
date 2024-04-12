add_library(library-cpp-svnversion)

target_link_libraries(library-cpp-svnversion PUBLIC
  yutil
  cpp-string_utils-misc
)

target_sources(library-cpp-svnversion PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/svnversion/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svnversion.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svn_interface.c
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/svnversion/svnversion.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/svnversion/svn_interface.c
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/svnversion/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svnversion.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/svnversion/svn_interface.c
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


